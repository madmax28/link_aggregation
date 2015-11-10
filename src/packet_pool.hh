#ifndef _PACKET_POOL_HH_
#define _PACKET_POOL_HH_

#include "common.hh"
#include "timer.hh"
#include "link.hh"

#include <cstdint>
#include <functional>
#include <algorithm>
#include <mutex>

class PacketPool {

    struct Packet {
        AlaggPacket *m_pkt = nullptr;
        int          m_size;

        void Set(AlaggPacket * p, int size) {
            m_pkt = p;
            m_size = size;
        }

        explicit operator bool() const {
//            std::cout << "Checking packet existence: " << (m_pkt != nullptr) << std::endl;
            return (m_pkt != nullptr);
        }
    };

    std::vector<Packet> m_packets;

    uint32_t m_timeout_msec;

    alagg_seq_t m_rx_seq;
    std::mutex m_flush_lock;

    /*
     * Access packets in pool by sequence no
     *   If the pool doesn't have a slot for the packet, it is resized
     */
    Packet& operator[](int const seq) {
        alagg_seq_t dist = SeqDistance(m_rx_seq, seq);
        if(dist > m_packets.size())
            m_packets.resize(dist);
        return m_packets[dist-1];
    }

    // Calculate distance from seq1 to seq2
    static alagg_seq_t SeqDistance(alagg_seq_t const from_seq,
                                   alagg_seq_t const to_seq) {

        if(from_seq <= to_seq) {
            return (to_seq - from_seq);
        } else {
            unsigned long long t = to_seq + ALAGG_MAX_SEQ;
            return (t - from_seq);
        }
    }

    /*
     * Flush until at least seq
     */

    static void Flush(PacketPool *t, const alagg_seq_t seq) {

        Packet p;

//        std::cout << "Flush until " << seq << " (m_rx_seq=" << t->m_rx_seq << ", pool_size=" << t->m_packets.size() << ")" << std::endl;

        // Sanity check
        // TODO DELETE
        if(SeqDistance(t->m_rx_seq, seq) > t->m_packets.size()) {
            return;
//            std::cerr << "Assertion failed. SeqDistance("
//                << t->m_rx_seq
//                << ", "
//                << seq
//                << ")="
//                << SeqDistance(t->m_rx_seq, seq)
//                << " > pool size="
//                << t->m_packets.size() << std::endl;
        }
//        assert(SeqDistance(t->m_rx_seq, seq) <= t->m_packets.size());

        while(t->m_rx_seq < seq) {

//            std::cout << "1. seq=" << ((t->m_rx_seq+1) % ALAGG_MAX_SEQ) << std::endl;
            p = (*t)[(t->m_rx_seq+1) % ALAGG_MAX_SEQ];

            // Packet present
            if(p) {

                // Pop it
//                std::cout << "Allocating Buffer for packet pool" << std::endl;
                Buffer *buf = new Buffer();
                buf->assign(((unsigned char *)p.m_pkt)+sizeof(AlaggHeader),
                            ((unsigned char *)p.m_pkt)+p.m_size);
//                std::cout << "First loop: Freeing a received packet (seq=" << p.m_pkt->m_header.m_seq << ")" << std::endl;
                free(p.m_pkt);
//                std::cout << "Free ok" << std::endl;
                t->PopPacketFromPool(buf);
            }

            // Remove popped element
            t->m_packets.erase(t->m_packets.begin());
            t->m_rx_seq++;
        }

        // Continue popping any successive, present packets
        p = (*t)[(t->m_rx_seq+1) % ALAGG_MAX_SEQ];
        while(p) {

//            std::cout << "2. seq=" << ((t->m_rx_seq+1) % ALAGG_MAX_SEQ) << std::endl;

            // Pop
//            std::cout << "Allocating Buffer for packet pool" << std::endl;
            Buffer *buf = new Buffer();
            buf->assign(((unsigned char *)p.m_pkt)+sizeof(AlaggHeader),
                        ((unsigned char *)p.m_pkt)+p.m_size);
//            std::cout << "Second loop: Freeing a received packet (seq=" << p.m_pkt->m_header.m_seq << ")" << std::endl;
            free(p.m_pkt);
//            std::cout << "Free ok" << std::endl;
            t->PopPacketFromPool(buf);

            // Remove popped element
            t->m_packets.erase(t->m_packets.begin());
            t->m_rx_seq++;

            p = (*t)[(t->m_rx_seq+1) % ALAGG_MAX_SEQ];
        }
    }

    static void FlushCb(PacketPool *t, const alagg_seq_t seq) {

        std::lock_guard<std::mutex> lock(t->m_flush_lock);

        t->Flush(t, seq);
    }

    virtual void PopPacketFromPool(Buffer * b) {
        std::cerr << "PopPacketFromPool() not overloaded?\n";
        delete b;
    }

    public:

    template<typename... A>
    PacketPool(uint32_t timeout_msec)
               : m_timeout_msec(timeout_msec)
               , m_rx_seq(0) {}

//    alagg_seq_t FirstSeq() { return m_packets[0].m_pkt->m_header.m_seq; }

    bool IsRecent( alagg_seq_t const seq ) const {
        return   ( (seq > m_rx_seq && (seq - m_rx_seq) < (ALAGG_MAX_SEQ/2))
                || (seq < m_rx_seq && (m_rx_seq - seq) > (ALAGG_MAX_SEQ/2)) );
    }

    void Add(AlaggPacket * p, int const size) {

        std::lock_guard<std::mutex> lock(m_flush_lock);

        // Ignore outdated packets
        if(!IsRecent(p->m_header.m_seq)) {
            free(p);
            return;
        }

        // Have packet already
        if((*this)[p->m_header.m_seq]) {
            free(p);
            return;
        }

        // Store packet
        (*this)[p->m_header.m_seq].Set(p, size);
        assert((*this)[p->m_header.m_seq]);

        /*
         * If the packet is in sequence, flush the pool immediately.
         * Otherwise, set a Timer to call Flush
         */
        if(p->m_header.m_seq == ((m_rx_seq+1) % ALAGG_MAX_SEQ)) {
            Flush(this, ((m_rx_seq+1) % ALAGG_MAX_SEQ));
        } else {
//            std::cout << "Timer for seq " << p->m_header.m_seq << std::endl;
            Timer(m_timeout_msec, FlushCb, this, p->m_header.m_seq);
        }
    }
};

#endif /* _PACKET_POOL_HH_ */

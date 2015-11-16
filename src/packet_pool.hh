/** @file packet_pool.hh
 * PacketPool class definition
 */

#ifndef _PACKET_POOL_HH_
#define _PACKET_POOL_HH_

#include "common.hh"
#include "timer.hh"
#include "link.hh"

#include <cstdint>
#include <functional>
#include <algorithm>
#include <mutex>

/**
 * PacketPool class
 *
 * Any packet received on the aggregated Links traverses the PacketPool. Packets
 * are stored here in a vector ordered by sequence number.
 * The PacketPool is also responsible for keeping track of the reception
 * sequence number.
 *
 * If a new packet is added, it is checked if the packet matches the expected
 * sequence number.
 * If so, the packet (and any possibly present successive
 * packets) are popped from the PacketPool via PopPacketFromPool, which is
 * supposed to be overloaded by classes inheriting from PacketPool.
 * If an added packet is out-of-order, it is added to the pool and it's delivery
 * is deferred. A timer is started for the packet. As soon as it times out, all
 * packets up to the sequence number of the original out-of-order packets are
 * delivered.
 *
 * @see Link
 * @see Timer
 */
class PacketPool {

    /**
     * Structure of a Packet in the Pool. Packets are store as a pointer and
     * their corresponding size. Pointers to packets are initialized as nullptr.
     */
    struct Packet {
        AlaggPacket *m_pkt = nullptr;
        int          m_size;

        /**
         * Assign a packet
         *
         * @param p Pointer to the new packet
         * @param size Size of the new packet
         */
        void Set(AlaggPacket * p, int size) {
            m_pkt = p;
            m_size = size;
        }

        /**
         * Overload a Packet's bool operator.
         *
         * @returns True if the packet is present, False otherwise.
         */
        explicit operator bool() const {
            return (m_pkt != nullptr);
        }
    };

    // Stored packets
    std::vector<Packet> m_packets;

    // Timeout for out-of-order packets
    uint32_t m_timeout_msec;

    // Rx sequence number
    alagg_seq_t m_rx_seq;

    // Protect access to the packet pool
    std::mutex m_ppool_lock;

    Packet& operator[](int const seq);
    static alagg_seq_t SeqDistance(alagg_seq_t const from_seq,
                                   alagg_seq_t const to_seq);
    static void Flush(PacketPool *t, const alagg_seq_t seq);
    static void FlushCb(PacketPool *t, const alagg_seq_t seq);
    virtual void PopPacketFromPool(Buffer * b);

    public:

    /**
     * PacketPool class constructor
     *
     * @param timeout_msec Time for which out-of-order packets are allowed to
     * be deferred before they are flushed.
     *
     * @see Timer
     */
    PacketPool(uint32_t timeout_msec)
               : m_timeout_msec(timeout_msec)
               , m_rx_seq(0) {}

    bool IsRecent( alagg_seq_t const seq ) const;
    void Add(AlaggPacket * p, int const size);
};

#endif /* _PACKET_POOL_HH_ */

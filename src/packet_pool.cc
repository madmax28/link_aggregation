#include "packet_pool.hh"

/**
 * Overload the classes subscript operator.
 *
 * This facilitates access to the packets stored in the pool. Packets are
 * accesses by their sequence number rather than the underlying vector's
 * index.
 * If access is done to a sequence number that is not covered by the pool,
 * the PacketPool's size is increased accordingly.
 *
 * @param seq Sequence number of the packets to be accesses.
 * @returns A reference to the Packet object in the pool.
 *
 * @see PacketPool::Packet
 */
PacketPool::Packet& PacketPool::operator[](int const seq) {

    // Calculate seq distance to requested packet
    alagg_seq_t dist = SeqDistance(m_rx_seq, seq);
    // Resize packet pool if necessary
    if(dist > m_packets.size())
        m_packets.resize(dist);
    return m_packets[dist-1];
}

/**
 * Calculate distance between two sequence numbers.
 *
 * @param from_seq Start sequence number.
 * @param to_seq End sequence number.
 * @returns Distance between from_seq and to_seq.
 */
alagg_seq_t PacketPool::SeqDistance(alagg_seq_t const from_seq,
                                           alagg_seq_t const to_seq) {

    if(from_seq <= to_seq) {
        return (to_seq - from_seq);
    } else {
        // SEQ wrapped
        unsigned long long t = to_seq + ALAGG_MAX_SEQ;
        return (t - from_seq);
    }
}

/**
 * Flush the PacketPool up to (at least) a given sequence number.
 *
 * Any packets in the pool are with a sequence number lesser than the given one
 * are flushed (in order).
 * In addition, any further in-sequence packets are already present in the pool,
 * are flushed as well.
 *
 * @param t Back-reference to calling PacketPool instance
 * @param seq Sequence number up to which is to be flushed
 * @see Timer
 */
void PacketPool::Flush(PacketPool *t, const alagg_seq_t seq) {

    Packet p;

    // Do nothing if sequence number is out of range
    if(SeqDistance(t->m_rx_seq, seq) > t->m_packets.size())
        return;

    // Flush until given given sequence number
    while(t->m_rx_seq < seq) {

        p = (*t)[(t->m_rx_seq+1) % ALAGG_MAX_SEQ];

        // Packet present?
        if(p) {

            // Pop it, without the AlaggHeader
            Buffer *buf = new Buffer();
            buf->assign(((unsigned char *)p.m_pkt)+sizeof(AlaggHeader),
                        ((unsigned char *)p.m_pkt)+p.m_size);
            free(p.m_pkt);
            t->PopPacketFromPool(buf);
        }

        // Remove popped element
        t->m_packets.erase(t->m_packets.begin());
        t->m_rx_seq++;
    }

    // Continue popping any successive, present packets
    p = (*t)[(t->m_rx_seq+1) % ALAGG_MAX_SEQ];
    while(p) {

        // Pop it, without the AlaggHeader
        Buffer *buf = new Buffer();
        buf->assign(((unsigned char *)p.m_pkt)+sizeof(AlaggHeader),
                    ((unsigned char *)p.m_pkt)+p.m_size);
        free(p.m_pkt);
        t->PopPacketFromPool(buf);

        // Remove popped element
        t->m_packets.erase(t->m_packets.begin());
        t->m_rx_seq++;

        p = (*t)[(t->m_rx_seq+1) % ALAGG_MAX_SEQ];
    }
}

/**
 * Wrapper for Timer callbacks that just locks the mutex and flushes.
 *
 * @param t Back-reference to calling PacketPool instance
 * @param seq Sequence number up to which is to be flushed
 */
void PacketPool::FlushCb(PacketPool *t, const alagg_seq_t seq) {

    std::lock_guard<std::mutex> lock(t->m_ppool_lock);
    t->Flush(t, seq);
}

/**
 * Dummy function.
 *
 * This function needs to be overloaded by the child class, and is called for
 * every packet that is removed from PacketPool.
 *
 * @param b Pointer to the Buffer object popped from PacketPool
 */
void PacketPool::PopPacketFromPool(Buffer * b) {
    std::cerr << __PRETTY_FUNCTION__ << " needs to be overloaded by child.\n";
    exit(-1);
}

/**
 * Check if a sequence number is recent.
 *
 * @param seq Sequence number to be checked.
 * @returns True if the sequence number is recent, false otherwise.
 */
bool PacketPool::IsRecent( alagg_seq_t const seq ) const {
    return   ( (seq > m_rx_seq && (seq - m_rx_seq) < (ALAGG_MAX_SEQ/2))
            || (seq < m_rx_seq && (m_rx_seq - seq) > (ALAGG_MAX_SEQ/2)) );
}

/**
 * Add a packet to the PacketPool.
 *
 * If the given packet is neither outdated nor alread present. The present is
 * added to PacketPool.
 * Furthermore, it is checked whether the packet's sequence number matches the
 * expected on (rx sequence number + 1).
 * If so, the Flush() routine is called immediately.
 * Otherwise, i.e. the packet is out-of-order, a new Timer object is created,
 * deferring the call to Flush() for m_timeout_msec milliseconds. This gives the
 * missing packet(s) a window to be still received, re-ordered, and delivered.
 *
 * @param p Pointer to the AlaggPacket to be added.
 * @param size Size of the packet.
 * @see Timer
 */
void PacketPool::Add(AlaggPacket * p, int const size) {

    std::lock_guard<std::mutex> lock(m_ppool_lock);

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
        Timer(m_timeout_msec, FlushCb, this, p->m_header.m_seq);
    }
}

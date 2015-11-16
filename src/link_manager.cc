#include "link_manager.hh"

/**
 * Link reception chain
 *
 * Polls on the aggregated links for reception readiness, and receives on links
 * in a round-robin fashion.
 * If an outdated packet is received on a Link, it is dropped and another
 * reception attempt is made on the same Link. This avoids degeneration of a
 * Link's 'freshness'.
 *
 * @param t Back-reference to the calling instance of LinkManager
 * @see PacketPool
 */
void LinkManager::recv_on_links(LinkManager *t) {

    AlaggPacket *packet;
    int byte_rcvd;
    int idx;

    // Poll on links
    int nfds_rdy = poll(t->m_link_pfds, t->m_link_nfds, -1);

    // Used for round-robin
    static unsigned int link_index = 0;

    // Allocate new buffer
    unsigned char *raw_buf = (unsigned char *) malloc(BUF_SIZE);

    // Loop over Links
    for( int i = 0; i < t->m_links.size(); i++ ) {

        // Check if link ready. If not, ask next link.
        if(!(t->m_link_pfds[link_index].revents & LINK_POLL_EVENTS)) {
                link_index = (link_index+1) % t->m_links.size();
                continue;
        }

        /*
         * Keep receiving on a link until a valid packet is received or no data
         * is available (EAGAIN, EWOULDBLOCK)
         */

        do {
            byte_rcvd = recv( t->m_links[link_index]->Socket(),
                    raw_buf,
                    BUF_SIZE, 0 );
            if( (errno == EAGAIN) || (errno == EWOULDBLOCK) ) {
                // No data avilable, continue on next link
                errno = 0;
                break;
            }
            assert_perror(errno);

            if( byte_rcvd == 0 ) {
                // Shouldn't happen
                std::cerr << "ERROR: Received 0 bytes" << std::endl;
                break;

            } else {

                /*
                 * Packet received
                 */

                packet = (AlaggPacket *) raw_buf;

                // Debug
                // print_buffer(buf.data(), buf.size());

                // Push the packet to the PacketPool
                t->Add(packet, byte_rcvd);
                link_index = (link_index+1) % t->m_links.size();
                return;
            }
        } while(true);

        link_index = (link_index+1) % t->m_links.size();
    }

    // No packet could be received, free the allocated buffer
    free(raw_buf);
}

/**
 * LinkManager class constructor.
 *
 * Initializes the aggregated links and starts the Link reception thread.
 *
 * @param peer_addresses Vector of the link peers' addresses as string.
 * @param if_names Vector of the interface names associated with the peers, as
 * strings.
 *
 * @see Link
 * @see SafeQueue
 * @see PipedThread
 * @see PacketPool
 */
LinkManager::LinkManager(std::vector<std::string> peer_addresses,
                         std::vector<std::string> if_names)
                         : PacketPool(ALAGG_REORDER_TTL)
                         , m_tx_seq(1) {

    // Initialize links
    for( int i = 0; i < peer_addresses.size(); i++ ) {
        m_links.push_back( new Link(if_names[i], peer_addresses[i]) );
    }

    // Start the reception thread
    SetThread(recv_on_links, this, PipedThread::exec_repeat);

    // Construct struct pollfd[] for polling, and set the related events.
    m_link_pfds = (struct pollfd *)
        malloc(m_links.size() * sizeof(struct pollfd));
    for(int i = 0; i < m_links.size(); i++) {
        m_link_pfds[i].fd = m_links[i]->Socket();
        m_link_pfds[i].events = LINK_POLL_EVENTS;
    }
    m_link_nfds = m_links.size();
}

/**
 * LinkManager class desctructor.
 *
 * Deallocates the associated Link objects.
 *
 * @see Link
 */
LinkManager::~LinkManager() {
    for(int i = 0; i < m_links.size(); i++) {
        delete m_links[i];
    }
}

/**
 * Link transmission.
 *
 * Send a packet via the aggregated links.
 *
 * @param buf Buffer object containing the packet to be sent.
 * @returns The return value of the underlying send() call.
 * @see Link
 */
int LinkManager::Send(Buffer const * buf) {

    int packet_size = sizeof(AlaggPacket) + buf->size();
    AlaggPacket *packet = (AlaggPacket *) malloc(packet_size);

    // Construct packet
    bzero(packet, packet_size);
    memcpy(packet->m_payload, buf->data(), buf->size());
    packet->m_header.m_eth_header.ether_type = ETH_P_ALAGG;
    packet->m_header.m_seq = NextTxSeq();

    // Loop over links
    for( int i = 0; i < m_links.size(); i++ ) {
        // Prepare ethernet header
        memcpy( packet->m_header.m_eth_header.ether_shost,
                m_links[i]->OwnAddr().Addr().data(),
                MAC_ADDRLEN );
        memcpy( packet->m_header.m_eth_header.ether_dhost,
                m_links[i]->PeerAddr().Addr().data(),
                MAC_ADDRLEN );

        send( m_links[i]->Socket(), packet, packet_size, 0 );
        errno = 0; assert_perror(errno);
    }

    free(packet);
    return 0;
}

/**
 * Link reception.
 *
 * Hook function to receive on the aggregated links. Note that actual link
 * reception is perform by LinkManager::recv_on_links(). After received packets
 * traverse the PacketPool, they are pushed to a SafeQueue object and then ready
 * for further processing.
 * This function simply performs SafeQueue::Pop() routine.
 *
 * @returns Pointer to a the Buffer objects containing the received packet, or
 * nullptr, if the the SafeQueue is empty.
 * @see SafeQueue
 * @see PacketPool
 */
Buffer const * LinkManager::Recv() {
    if(Empty()) {
        return nullptr;
    } else {
        Buffer *buf = Front();
        Pop();
        // Read a byte from the pipe stream
        EmptyPipe();
        return buf;
    }
}


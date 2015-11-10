#include "link_manager.hh"

void LinkManager::recv_on_links(LinkManager *t) {

    /*
     * Poll on links
     */

    int nfds_rdy = poll(t->m_link_pfds, t->m_link_nfds, -1);

    // Used for round-robin
    static unsigned int link_index = 0;

//    std::cout << "Allocating buffer for reception" << std::endl;
    unsigned char *raw_buf = (unsigned char *) malloc(BUF_SIZE);
    AlaggPacket *packet;
    int byte_rcvd;
    int idx;

    for( int i = 0; i < t->m_links.size(); i++ ) {

        if(!(t->m_link_pfds[link_index].revents & LINK_POLL_EVENTS)) {
                link_index = (link_index+1) % t->m_links.size();
                continue;
        }

        /*
         * Keep receiving on a link until a valid packet is received or no data
         * is available (EAGAIN, EWOULDBLOCK)
         */

//        std::cout << "link " << link_index << std::endl;

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

                /*
                 * If the packet is not in sequence, drop it.
                 * Else, accept it and update rx seq.
                 */

//                std::cout << "Received packet " << packet->m_header.m_seq;

//                if( !t->IsRecent(packet->m_header.m_seq) ) {
//                    std::cout << ".. and dropped it" << std::endl;
//                    continue;
//                }

//                std::cout << std::endl;

                /*
                 * We do not return the Ethernet header
                 *   This is removed before delivering packets to client
                 */
                // TODO : do this in pop func

//                Buffer *buf = new Buffer();
//                buf->assign(raw_buf+sizeof(AlaggHeader), raw_buf+byte_rcvd);

                // Debug
//                print_buffer(buf.data(), buf.size());

                t->Add(packet, byte_rcvd);
                link_index = (link_index+1) % t->m_links.size();
                return;
            }
        } while(true);

        link_index = (link_index+1) % t->m_links.size();
    }

//    std::cout << "Freeing a receive buffer" << std::endl;
    free(raw_buf);
//    std::cout << "Free ok" << std::endl;
}

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

    /*
     * Construct struct pollfd[] for polling, and set the related events.
     */

    m_link_pfds = (struct pollfd *)
        malloc(m_links.size() * sizeof(struct pollfd));
    for(int i = 0; i < m_links.size(); i++) {
        m_link_pfds[i].fd = m_links[i]->Socket();
        m_link_pfds[i].events = LINK_POLL_EVENTS;
    }
    m_link_nfds = m_links.size();
}

LinkManager::~LinkManager() {
    for(int i = 0; i < m_links.size(); i++) {
        delete m_links[i];
    }
}

int LinkManager::Send(Buffer const * buf) {

    int packet_size = sizeof(AlaggPacket) + buf->size();
    AlaggPacket *packet = (AlaggPacket *) malloc(packet_size);

    // Construct packet
    bzero(packet, packet_size);
    memcpy(packet->m_payload, buf->data(), buf->size());
    packet->m_header.m_eth_header.ether_type = ETH_P_ALAGG;
    packet->m_header.m_seq = NextTxSeq();

    for( int i = 0; i < m_links.size(); i++ ) {
//        std::cout << "Sending " << packet->m_header.m_seq << " on link " << i << std::endl;
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


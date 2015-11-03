#include "link_manager.hh"

void LinkManager::recv_on_links(LinkManager *t) {

    std::cout << "LinkManager::recv()\n";

    // Used for round-robin
    static unsigned int link_index = 0;

    Buffer buf;
    char raw_buf[BUF_SIZE];
    AlaggPacket *packet;
    int byte_rcvd;
    int idx;

    for( int i = 0; i < t->m_links.size(); i++ ) {

        /*
         * Keep receiving on a link until a valid packet is received or no data
         * is available (EAGAIN, EWOULDBLOCK)
         */

        std::cout << "link " << link_index << std::endl;

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
                goto done;

            } else {

                /*
                 * Packet received
                 */

                packet = (AlaggPacket *) raw_buf;

                /*
                 * If the packet is not in sequence, drop it.
                 * Else, accept it and update rx seq.
                 */

                std::cout << "Received packet " << packet->m_header.m_seq;

                if( !t->IsInSequence(packet->m_header.m_seq) ) {
                    std::cout << ".. and dropped it" << std::endl;
                    continue;
                }

                std::cout << std::endl;

                // Update SEQ
                // t->m_rx_seq = (++t->m_rx_seq % (ALAGG_MAX_SEQ));
                t->m_rx_seq = packet->m_header.m_seq;

                /*
                 * We do not return the Ethernet header
                 *   This is removed before delivering packets to client
                 */

                buf.insert( buf.end(),
                        raw_buf+sizeof(AlaggHeader),
                        raw_buf+byte_rcvd );
                goto done;

                // Debug
                //            print_packet( (AlaggPacket *) raw_buf, buf.size() );
            }
        } while(true);

                link_index = (link_index+1) % t->m_links.size();
    }

done:
    link_index = (link_index+1) % t->m_links.size();
    t->Push(buf);
}

bool LinkManager::IsInSequence( unsigned short const seq ) const {
    return   ( (seq > m_rx_seq && (seq - m_rx_seq) < (ALAGG_MAX_SEQ/2))
            || (seq < m_rx_seq && (m_rx_seq - seq) > (ALAGG_MAX_SEQ/2)) );
}

LinkManager::LinkManager(std::vector<std::string> peer_addresses,
                         std::vector<std::string> if_names)
                         : m_rx_seq(0)
                         , m_tx_seq(1) {

    // Initialize links
    for( int i = 0; i < peer_addresses.size(); i++ ) {
        m_links.push_back( new Link(if_names[i], peer_addresses[i]) );
    }

    // Start the reception thread
    SetThread(recv_on_links, this, PipedThread::exec_repeat);
}

LinkManager::~LinkManager() {
    for(int i = 0; i < m_links.size(); i++) {
        delete m_links[i];
    }
}

int LinkManager::Send(Buffer const & buf) {

    int packet_size = sizeof(AlaggPacket) + buf.size();
    AlaggPacket *packet = (AlaggPacket *) malloc(packet_size);

    // Construct packet
    bzero(packet, packet_size);
    memcpy(packet->m_payload, buf.data(), buf.size());
    packet->m_header.m_eth_header.ether_type = ETH_P_ALAGG;
    packet->m_header.m_seq = NextTxSeq();

    for( int i = 0; i < m_links.size(); i++ ) {
        std::cout << "Sending " << packet->m_header.m_seq << " on link " << i << std::endl;
        // Prepare ethernet header
        memcpy( packet->m_header.m_eth_header.ether_shost,
                m_links[i]->OwnAddr().Addr().data(),
                MAC_ADDRLEN );
        memcpy( packet->m_header.m_eth_header.ether_dhost,
                m_links[i]->PeerAddr().Addr().data(),
                MAC_ADDRLEN );

        int byte_sent = send( m_links[i]->Socket(), packet, packet_size, 0 );
        if( byte_sent == -1 ) {
            perror("send()");
            exit(1);
        }
    }

    free(packet);
    return 0;
}

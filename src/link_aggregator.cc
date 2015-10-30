#include <iostream>
#include <unistd.h> // sleep()
#include <string.h> // memcpy()
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include "link_aggregator.hh"

void print_packet( AlaggPacket *packet, const unsigned int size );

LinkAggregator::LinkAggregator( const std::string config_filename )
        : m_config(config_filename)
        , m_tx_seq(1)
        , m_rx_seq(0) {

    // Init Links
    std::vector<std::string> peer_addresses = m_config.PeerAddresses();
    std::vector<std::string> if_names       = m_config.IfNames();

    for( int i = 0; i < peer_addresses.size(); i++ ) {
        m_links.push_back( new Link(if_names[i], peer_addresses[i]) );
    }

    // Print config
    PrintConfig();
}

Buffer LinkAggregator::RecvPktFromClient() {

    return m_client.RecvPkt();
}

int LinkAggregator::SendPktToClient( Buffer const &buf ) {

    return m_client.SendPkt(buf);
}

unsigned short LinkAggregator::NextTxSeq() {
    return (m_tx_seq++ % USHRT_MAX);
}

bool LinkAggregator::IsInSequence( unsigned short const seq ) const {
    return   ( (seq > m_rx_seq && (seq - m_rx_seq) < (USHRT_MAX/2))
            || (seq < m_rx_seq && (m_rx_seq - seq) > (USHRT_MAX/2)) );
}

void LinkAggregator::PrintConfig() const {
    std::cout << "Proxying traffic destined for:\n";
    std::cout << "    " << m_config.ClientIp().Str() << std::endl;
    std::cout << "Link setup:\n";
    for( int i = 0; i < m_links.size(); i++ ) {
        std::cout << "    ";
        std::cout << m_links[i]->OwnAddr().Str();
        std::cout << " <--" << m_links[i]->IfName() << "--> ";
        std::cout << m_links[i]->PeerAddr().Str();
        std::cout << std::endl;
    }
}

/*
 * Send the provided msg on all links
 */

int LinkAggregator::SendOnLinks( Buffer const &buf ) {

    int packet_size = sizeof(AlaggPacket) + buf.size();
    AlaggPacket *packet = (AlaggPacket *) malloc( packet_size );

    // Construct packet
    bzero( packet, packet_size );
    memcpy( packet->m_payload, buf.data(), buf.size() );
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

/*
 * Receive a single packet from the links.
 *   Round-robin scheduling is used to select the link to receive on.
 *   If an old packet was received on a link, it is dropped and attempted to
 *   receive another packet on that link.
 *   If no data is ready to be received on a link, the next link is picked for
 *   reception.
 *   The function returns as soon as a valid packet is received, or no data is
 *   available at any of the links.
 */

Buffer const LinkAggregator::RecvOnLinks() {

    // Used for round-robin
    static unsigned int link_index = 0;

    Buffer buf;
    char raw_buf[BUF_SIZE];
    AlaggPacket *packet;
    int byte_rcvd;
    int idx;

    for( int i = 0; i < m_links.size(); i++ ) {

        /*
         * Keep receiving on a link until a valid packet is received or no data
         * is available (EAGAIN, EWOULDBLOCK)
         */

        std::cout << "link " << link_index << std::endl;

        do {
            byte_rcvd = recv( m_links[link_index]->Socket(),
                    raw_buf,
                    BUF_SIZE, 0 );

            if( byte_rcvd == -1 ) {
                if( (errno == EAGAIN) || (errno == EWOULDBLOCK) ) {
                    // No data avilable, continue on next link
                    break;
                } else {
                    // Error
                    perror("recv()");
                    goto done;
                }

            } else if( byte_rcvd == 0 ) {
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

                if( !IsInSequence(packet->m_header.m_seq) ) {
                    std::cout << ".. and dropped it" << std::endl;
                    continue;
                }

                std::cout << std::endl;

                // Update SEQ
//                m_rx_seq = (++m_rx_seq % (USHRT_MAX));
                m_rx_seq = packet->m_header.m_seq;

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

        link_index = (link_index+1) % m_links.size();
    }

done:
    link_index = (link_index+1) % m_links.size();
    return buf;

}

void print_mac( const unsigned char *mac )
{
    int i;

    for( i = 0; i < ETH_ALEN; i++ ) {
        printf( "%02x%s", mac[i], (i == ETH_ALEN-1 ? "" : ":") );
    }
}

void print_packet( AlaggPacket *packet, const unsigned int size ) {

    int it;

    printf( "Packet =================================================\n" );
    printf( "Content:\n" );
    for( it=0; it<size; it++ ) {
        unsigned char c = ((unsigned char*) packet)[it];
        printf( "%02X ", c );
        if( !((it + 1) %  4 ) ) printf("   ");
        if( !((it + 1) % 16 ) ) printf("\n");
    }
    printf("\n");
    printf( "From         : " );
    print_mac( packet->m_header.m_eth_header.ether_shost );
    printf("\n");
    printf( "To           : " );
    print_mac( packet->m_header.m_eth_header.ether_dhost );
    printf("\n");
    printf( "Eth type     : 0x%x\n", ntohs(packet->m_header.m_eth_header.ether_type) );
    printf( "========================================================\n" );
}

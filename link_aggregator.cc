#include <iostream>
#include <unistd.h> // sleep()
#include <string.h> // memcpy()
#include <stdlib.h>
#include <errno.h>

#include "link_aggregator.h"

void print_packet( AlaggPacket *packet, const unsigned int size );

LinkAggregator::LinkAggregator( const std::string config_filename )
        : m_config(config_filename) {

    // Init Links
    std::vector<std::string> peer_addresses = m_config.PeerAddresses();
    std::vector<std::string> if_names       = m_config.IfNames();

    for( int i = 0; i < peer_addresses.size(); i++ ) {
        m_links.push_back( new Link(if_names[i], peer_addresses[i]) );
    }
}

Buffer LinkAggregator::RecvPktFromClient() {

    return m_client.RecvPkt();
}

int LinkAggregator::SendPktToClient( Buffer const &buf ) const {

    return m_client.SendPkt(buf);
}

/*
 * Send the provided msg on all links
 */

int LinkAggregator::SendOnLinks( Buffer const &buf ) const {

    int packet_size = sizeof(AlaggPacket) + buf.size();
    AlaggPacket *packet = (AlaggPacket *) malloc( packet_size );

    // Construct packet
    bzero( packet, packet_size );
    memcpy( packet->m_payload, buf.data(), buf.size() );
    packet->m_header.m_eth_header.ether_type = ETH_P_ALAGG;

    for( int i = 0; i < m_links.size(); i++ ) {
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
 * Receive a single packet on any of the links
 */

Buffer const LinkAggregator::RecvOnLinks() const {

    // TODO: round robin on links

    Buffer buf;
    char raw_buf[BUF_SIZE];
    AlaggPacket *packet;
    int byte_rcvd;

    for( int i = 0; i < m_links.size(); i++ ) {
        byte_rcvd = recv( m_links[i]->Socket(), raw_buf, BUF_SIZE, 0 );
        if( byte_rcvd == -1 ) {
            if( (errno != EAGAIN) && (errno != EWOULDBLOCK) ) {
                perror("recv()");
                return buf;
            }
        } else if( byte_rcvd == 0 ) {
            std::cerr << "ERROR: Received 0 bytes" << std::endl;
            return buf;
        } else {
            /*
             * We do not return the Ethernet header
             *   This is removed before handing packets to client
             */

            packet = (AlaggPacket *) raw_buf;
            // TODO stuff with packet here

            buf.insert( buf.end(), raw_buf+sizeof(AlaggHeader), raw_buf+byte_rcvd );

            // Debug
//            print_packet( (AlaggPacket *) raw_buf, buf.size() );
        }
    }

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

#include <iostream>
#include <unistd.h> // sleep()
#include <string.h> // memcpy()
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include "link_aggregator.hh"

LinkAggregator::LinkAggregator( const std::string config_filename )
        : m_config(config_filename)
        , m_link_manager(m_config.PeerAddresses(), m_config.IfNames()) {

    // Print config
    PrintConfig();
}

Buffer LinkAggregator::RecvPktFromClient() {

    return m_client.RecvPkt();
}

int LinkAggregator::SendPktToClient( Buffer const &buf ) {

    return m_client.SendPkt(buf);
}

void LinkAggregator::PrintConfig() const {
    std::cout << "Proxying traffic destined for:\n";
    std::cout << "    " << m_config.ClientIp().Str() << std::endl;
    std::cout << "Link setup:\n";
    auto links = m_link_manager.Links();
    for( int i = 0; i < links.size(); i++ ) {
        std::cout << "    ";
        std::cout << links[i]->OwnAddr().Str();
        std::cout << " <--" << links[i]->IfName() << "--> ";
        std::cout << links[i]->PeerAddr().Str();
        std::cout << std::endl;
    }
}

/*
 * Send the provided msg on all links
 */

int LinkAggregator::SendOnLinks( Buffer const &buf ) {

    return m_link_manager.Send(buf);
}

Buffer const LinkAggregator::RecvOnLinks() {

    return m_link_manager.Recv();
}

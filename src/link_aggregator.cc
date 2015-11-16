#include <iostream>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>

#include "link_aggregator.hh"

/**
 * LinkAggregator class constructor
 *
 * Constructs a new LinkAggregator object. Constructs the corresponding Client
 * and LinkManager classes, and collects the file descriptors necessary to
 * perform asynchronous I/O via poll().
 *
 * @param config_filename Name of the configuration file to be used. If argument
 * is not given "default_config.cfg" is used.
 * @see Config
 * @see Client
 * @see LinkManager
 */
LinkAggregator::LinkAggregator( const std::string config_filename )
        : m_config(config_filename)
        , m_link_manager(m_config.PeerAddresses(), m_config.IfNames())
        , m_nfds(2) {

    m_pfds[0].fd = m_link_manager.PipeRxFd();
    m_pfds[1].fd = m_client.RxFd();
    m_pfds[0].events = LAGG_POLL_EVENTS;
    m_pfds[1].events = LAGG_POLL_EVENTS;

    // Print config
    PrintConfig();
}

/**
 * Perform link aggregation.
 *
 * Starts the aggregation loop. In every iteration, it is poll()'d for reception
 * from the Client and the LinkManager, and the corresponding event is handled.
 *
 * @see Client
 * @see LinkManager
 */
void LinkAggregator::Aggregate() {

    for(;;) {
        errno = 0;
        int nfds_rdy = poll(m_pfds, m_nfds, -1);
        assert_perror(errno);

        if(m_pfds[1].revents & LAGG_POLL_EVENTS) {
            TransmissionChain();
        }

        if(m_pfds[0].revents & LAGG_POLL_EVENTS) {
            ReceptionChain();
        }
    }
}

/**
 * Transmission chain.
 *
 * Receives a packet from the Client class and hands it to the LinkManager
 * class.
 *
 * @see Client
 * @see LinkManager
 */
void LinkAggregator::TransmissionChain() {

    Buffer const * buf;

    // Receive from client
    buf = RecvPktFromClient();
    if(buf) {
        // Got packet, forward to links
        SendOnLinks(buf);
        delete buf;
    }
}

/**
 * Reception chain.
 *
 * Receives a packet from the LinkManager class and delivers it to the Client
 * class.
 *
 * @see Client
 * @see LinkManager
 */
void LinkAggregator::ReceptionChain() {

    Buffer const * buf;

    // Receive from links
    buf = RecvOnLinks();
    if(buf) {
        // Got packet, forward to client
        SendPktToClient(buf);
        delete buf;
    }
}

/**
 * Client packet reception.
 *
 * Receive a packet from the client via the Client class.
 *
 * @returns A newly allocated Buffer object containing the received packet.
 * @see Client
 */
Buffer const * LinkAggregator::RecvPktFromClient() {

    return m_client.RecvPkt();
}

/**
 * Client packet transmission.
 *
 * Send a packet to the client via the Client class.
 *
 * @param buf A Buffer object containing the data to be sent.
 * @returns The return value of the underlying send() call.
 * @see Client
 */
int LinkAggregator::SendPktToClient( Buffer const * buf ) {

    return m_client.SendPkt(buf);
}

/**
 * Print the applications configuration to stdout
 */
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

/**
 * Link transmission
 *
 * Send a packet on the aggregated links via LinkManager.
 *
 * @param buf Buffer object containing the data to be sent.
 * @returns The return value of the underlying send() call.
 * @see LinkManager
 * @see Link
 */
int LinkAggregator::SendOnLinks( Buffer const * buf ) {

    return m_link_manager.Send(buf);
}

/**
 * Link reception
 *
 * Receives a packet from the aggregated links via LinkManager.
 *
 * @returns A newly allocated Buffer object containing the received packet.
 * @see LinkManager
 * @see Link
 */
Buffer const * LinkAggregator::RecvOnLinks() {

    return m_link_manager.Recv();
}

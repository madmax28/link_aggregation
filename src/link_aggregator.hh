/** @file link_aggregator.hh
 * LinkAggregator class definition
 */

#ifndef _LAGG_HH_
#define _LAGG_HH_

#include "config.hh"
#include "client.hh"
#include "common.hh"
#include "link_manager.hh"

#include <poll.h>
#include <vector>

/**
 * Definition of events that will be polled on using poll()
 *
 * The events include any incoming traffic.
 */
#define LAGG_POLL_EVENTS (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI)

/**
 * LinkAggregator class
 *
 * LinkAggregtor is the main class managing the aggregation. It includes an
 * instance of Client, which handles client connections and an instance of
 * LinkManager, which handles the aggregated links.
 *
 * This class mainly moves data between the LinkManager and the Client.
 *
 * @see Config
 * @see Client
 * @see LinkManager
 */
class LinkAggregator {

    Config      m_config;
    Client      m_client;
    LinkManager m_link_manager;

    // Used for asynchronous I/O on Client and Link reception
    struct pollfd m_pfds[2];
    nfds_t        m_nfds;

    private:

    void PrintConfig() const;

    public:

    LinkAggregator( const std::string config_filename = "default_config.cfg" );

    // Main operation loop
    void Aggregate();

    // TX and RX chain
    void TransmissionChain();
    void ReceptionChain();

    // Client communication
    Buffer const * RecvPktFromClient();
    int SendPktToClient( Buffer const * buf );

    // Link communication
    Buffer const * RecvOnLinks();
    int SendOnLinks( Buffer const * buf );
};

#endif /* _LAGG_HH_ */

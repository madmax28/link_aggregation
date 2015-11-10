#ifndef _LAGG_HH_
#define _LAGG_HH_

#include "config.hh"
#include "link.hh"
#include "client.hh"
#include "common.hh"
#include "safe_queue.hh"
#include "piped_thread.hh"
#include "link_manager.hh"
#include "packet_pool.hh"

#include <poll.h>
#include <vector>

#define LAGG_POLL_EVENTS (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI)

class LinkAggregator {

    Config      m_config;
    Client      m_client;
    LinkManager m_link_manager;

    // Used for asynchronous I/O on Client and Link (via LinkManager) reception
    struct pollfd m_pfds[2];
    nfds_t        m_nfds;

    private:

    void PrintConfig() const;

    public:

    LinkAggregator( const std::string config_filename = "default_config.cfg" );

    // Main operation loop
    void Aggregate();

    void DeliveryChain();
    void ReceptionChain();

    Buffer const * RecvPktFromClient();
    int SendPktToClient( Buffer const * buf );

    Buffer const * RecvOnLinks();
    int SendOnLinks( Buffer const * buf );
};

#endif /* _LAGG_HH_ */

#ifndef _LAGG_HH_
#define _LAGG_HH_

#include "config.hh"
#include "link.hh"
#include "client.hh"
#include "common.hh"
#include "safe_queue.hh"
#include "piped_thread.hh"
#include "link_manager.hh"

#include <vector>

class LinkAggregator {

    Config      m_config;
    Client      m_client;
    LinkManager m_link_manager;

    private:

    void PrintConfig() const;

    public:

    LinkAggregator( const std::string config_filename = "default_config.cfg" );

    Buffer * RecvPktFromClient();
    int SendPktToClient( Buffer const * buf );

    Buffer * RecvOnLinks();
    int SendOnLinks( Buffer const * buf );
};

#endif /* _LAGG_HH_ */

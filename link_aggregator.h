#ifndef _LAGG_H_
#define _LAGG_H_

#include <vector>
#include "config.h"
#include "link.h"
#include "client.h"
#include "common.h"

class LinkAggregator {

    Config              m_config;
    Client              m_client;
    std::vector<Link *> m_links;

    public:

    LinkAggregator( const std::string config_filename = "default_config.cfg" );

    Buffer RecvPktFromClient();
    int SendPktToClient( Buffer const &buf ) const;

    Buffer const RecvOnLinks() const;
    int SendOnLinks( Buffer const &buf ) const;
};

#endif /* _LAGG_H_ */

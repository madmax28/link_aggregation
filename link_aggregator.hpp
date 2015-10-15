#ifndef _LAGG_HPP_
#define _LAGG_HPP_

#include <vector>
#include "link.hpp"
#include "client.hpp"
#include "common.hpp"

class LinkAggregator {

    /*
     * Configuration parameters
     */

    /* Client related stuff */

    // Port for communication with client app
    int m_clientport;
    Client m_client;

    /* Link aggregation related stuff */

    // Aggregated links
    std::vector<Link> m_links;

    public:

    LinkAggregator(int num_links = 2);

    Buffer RecvPktFromClient() {
        return m_client.RecvPkt();
    }
};

#endif /* _LAGG_HPP_ */

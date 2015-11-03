#ifndef _LAGG_HH_
#define _LAGG_HH_

#include "config.hh"
#include "link.hh"
#include "client.hh"
#include "common.hh"
#include "safe_queue.hh"
#include "piped_thread.hh"
#include "link_receptor.hh"

#include <vector>

class LinkAggregator {

    Config                   m_config;
    Client                   m_client;

    // Stores reordered packets ready to be delivered to the client
//    SafeQueue<AlaggPacket &> m_pkt_q;
//    PipedThread              m_links_rx_t;
    std::vector<Link *>      m_links;

    unsigned short           m_tx_seq;
    unsigned short           m_rx_seq;

    private:

    unsigned short NextTxSeq();
    bool IsInSequence( unsigned short const seq ) const;

    void PrintConfig() const;

    public:

    LinkAggregator( const std::string config_filename = "default_config.cfg" );

    Buffer RecvPktFromClient();
    int SendPktToClient( Buffer const &buf );

    Buffer const RecvOnLinks();
    int SendOnLinks( Buffer const &buf );
};

#endif /* _LAGG_HH_ */

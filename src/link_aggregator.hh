#ifndef _LAGG_HH_
#define _LAGG_HH_

#include <vector>
#include "config.hh"
#include "link.hh"
#include "client.hh"
#include "common.hh"

class LinkAggregator {

    Config              m_config;
    Client              m_client;
    std::vector<Link *> m_links;

    unsigned short      m_tx_seq;
    unsigned short      m_rx_seq;

    // To store out-of-order packets
    std::vector<AlaggPacket> m_pkt_buf;

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

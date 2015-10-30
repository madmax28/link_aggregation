#ifndef _CONFIG_HH_
#define _CONFIG_HH_

#include <string>
#include <vector>

#include "common.hh"

class Config {

    IpAddress m_destination_ip;
    std::vector<std::string> m_peer_addresses;
    std::vector<std::string> m_if_names;

    void ReadConfig( std::string filename );

    public:
        Config( std::string filename );

        IpAddress const & ClientIp() const { return m_destination_ip; }
        std::vector<std::string> const PeerAddresses() const {
            return m_peer_addresses;
        }
        std::vector<std::string> const IfNames() const {
            return m_if_names;
        }
};

#endif /* _CONFIG_HH_ */

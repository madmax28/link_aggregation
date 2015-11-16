/** @file config.hh
 * Config class definition
 */

#ifndef _CONFIG_HH_
#define _CONFIG_HH_

#include <string>
#include <vector>

#include "common.hh"

/**
 * Config class
 *
 * Reads a configuration file and stores it's parameters.
 */
class Config {

    IpAddress m_destination_ip;
    std::vector<std::string> m_peer_addresses;
    std::vector<std::string> m_if_names;

    void ReadConfig( std::string filename );

    public:

        Config( std::string filename );

        /**
         * Getter for proxy destination IP
         *
         * @returns An IpAddress object containing the proxy's destination IP.
         */
        IpAddress const & ClientIp() const { return m_destination_ip; }

        /**
         * Getter for link peer addresses.
         *
         * @returns A vector of strings containing the peers' MAC addresses
         */
        std::vector<std::string> const PeerAddresses() const {
            return m_peer_addresses;
        }

        /**
         * Getter for names of interfaces to bind to.
         *
         * @returns A vector of strings containing the interface names
         * corresponding the the peers
         */
        std::vector<std::string> const IfNames() const {
            return m_if_names;
        }
};

#endif /* _CONFIG_HH_ */

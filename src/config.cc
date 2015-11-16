#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>

#include "config.hh"

/**
 * Constructs a new Config object.
 *
 * Reads the provided file and initializes it's members accordingly.
 *
 * @param filename Name of the configuration file to be loaded.
 */
Config::Config( std::string filename ) {
    ReadConfig(filename);
}

/**
 * Sets the configuration to match the one of the provided file.
 *
 * @param filename Name of the configuration file to be loaded.
 */
void Config::ReadConfig( std::string filename ) {

    std::ifstream in;
    std::string line;
    const std::string delimeter = "=";

    // Open file
    in.open(filename.c_str());
    if( in.fail() ) {
        std::cerr << "Error opening file: \"" << filename << "\"" << std::endl;
        exit(1);
    }

    std::vector<std::string> peer_addresses;
    std::vector<std::string> peer_interfaces;

    // Parse line by line
    while( std::getline( in, line ) ) {
        std::string token, value;

        token = line.substr( 0, line.find(delimeter) );
        value = line.substr( line.find(delimeter)+1, *line.rbegin() );

        // Dest ip
        if( token == "destination_ip" ) {
            m_destination_ip.SetIp(value);

        // Link peer addr
        } else if( token == "link_peers" ) {
            // Addresses are delimited by spaces
            std::size_t needle = value.find(" ");
            while( needle != std::string::npos ) {
                m_peer_addresses.push_back( value.substr( 0, needle ) );
                value = value.substr( needle+1, *value.rbegin() );
                needle = value.find(" ");
            }
            m_peer_addresses.push_back( value );

        // Link if name
        } else if( token == "link_if_names" ) {
            // Addresses are delimited by spaces
            std::size_t needle = value.find(" ");
            while( needle != std::string::npos ) {
                m_if_names.push_back( value.substr( 0, needle ) );
                value = value.substr( needle+1, *value.rbegin() );
                needle = value.find(" ");
            }
            m_if_names.push_back( value );
        }
    }

    // Verify number of interfaces/links
    if( m_if_names.size() != m_peer_addresses.size() ) {
        std::cerr << "ERROR: Number of interfaces does not match"
            << " number of mac addresses"
            << std::endl;
        exit(1);
    }
}

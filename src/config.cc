#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>

#include "config.hh"

Config::Config( std::string filename ) {
    ReadConfig(filename);
}

void Config::ReadConfig( std::string filename ) {

    std::ifstream in;
    std::string line;
    const std::string delimeter = "=";

    in.open(filename.c_str());
    if( in.fail() ) {
        std::cerr << "Error opening file: \"" << filename << "\"" << std::endl;
        exit(1);
    }

    std::vector<std::string> peer_addresses;
    std::vector<std::string> peer_interfaces;

    while( std::getline( in, line ) ) {
        std::string token, value;

//        std::cout << line << std::endl;
//        std::cout << "token="
//            << line.substr( 0, line.find(delimeter) )
//            << std::endl;
//        std::cout << "value="
//            << line.substr( line.find(delimeter)+1, *line.rbegin() )
//            << std::endl;

        token = line.substr( 0, line.find(delimeter) );
        value = line.substr( line.find(delimeter)+1, *line.rbegin() );

        if( token == "destination_ip" ) {
            m_destination_ip.SetIp(value);

        } else if( token == "link_peers" ) {
            // Addresses are delimited by spaces
            std::size_t needle = value.find(" ");
            while( needle != std::string::npos ) {
                m_peer_addresses.push_back( value.substr( 0, needle ) );
                value = value.substr( needle+1, *value.rbegin() );
                needle = value.find(" ");
            }
            m_peer_addresses.push_back( value );

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

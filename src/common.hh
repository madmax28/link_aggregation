#ifndef _COMMON_HH_
#define _COMMON_HH_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <iostream>

#include <net/if.h>

#include <arpa/inet.h>
#include <vector>

#define BUF_SIZE    4096
#define IP_ADDR_LEN 4

#define IFNAME_LOOPBACK "lo"

#define IP_HEADER_OFFSET 0
#define IP_HEADER_DST_ADDR_OFFSET 16

#define MAC_ADDRLEN 6
#define MAC_ADDR_STRLEN 17

typedef std::vector<unsigned char> Buffer;

class IpAddress {
    std::string    m_str;
    struct in_addr m_addr;

    public:
    IpAddress() {}
    IpAddress( std::string str ) {
        m_str = str;
        inet_aton( m_str.c_str(), &m_addr );
    }

    void SetIp( std::string str ) {
        if( inet_aton( str.c_str(), &m_addr ) != 1 ) {
            std::cerr << "Invalid address: " << str << std::endl;
            exit(1);
        }
        m_str = str;
    }

    std::string const & Str() const { return m_str; }
    struct in_addr const & Addr() const { return m_addr; }
};

class MacAddress {
    std::string       m_str;
    std::vector<char> m_addr;

    public:

    MacAddress() { }
    MacAddress( std::string const str ) {
        SetAddr(str);
    }

    void SetAddr( std::string const str ) {
        m_str = str;
        m_addr.resize(MAC_ADDRLEN);
        sscanf( str.c_str(),
                "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
                &m_addr[0],
                &m_addr[1],
                &m_addr[2],
                &m_addr[3],
                &m_addr[4],
                &m_addr[5] );
//        std::cout << m_str << std::endl;
    }

    std::string const Str() const { return m_str; }
    std::vector<char> const Addr() const { return m_addr; }
};

#endif /* _COMMON_HH_ */

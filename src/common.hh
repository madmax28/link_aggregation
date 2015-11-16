/** @file common.hh
 * Common functionalities and definitions.
 */

#ifndef _COMMON_HH_
#define _COMMON_HH_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <iostream>

#include <net/if.h>
#include <linux/if_ether.h>

#include <arpa/inet.h>
#include <vector>

/**
 * Maximum length buffers used.
 *
 * May be increased if necessary.
 */
#define BUF_SIZE    4096

/**
 * Name of the loopback interface
 */
#define IFNAME_LOOPBACK "lo"

/**
 * Number of bytes in an IP address.
 */
#define IP_ADDR_LEN 4
/**
 * Offset in bytes to where the IP header starts
 */
#define IP_HEADER_OFFSET 0
/**
 * Offset in bytes to where the IP destination addres starts within the IP
 * header
 */
#define IP_HEADER_DST_ADDR_OFFSET 16

/**
 * Length of a MAC address in bytes.
 */
#define MAC_ADDRLEN 6
/**
 * Length of a MAC address string in bytes.
 */
#define MAC_ADDR_STRLEN 17

/**
 * Type definition of the Buffer object used throughout the project.
 */
typedef std::vector<unsigned char> Buffer;

/**
 * Class to manage an IP address
 *
 * Storange class to holds an IP address as both a human-readable string as well
 * as a struct in_addr.
 */
class IpAddress {

    std::string    m_str;
    struct in_addr m_addr;

    public:

    /**
     * Construct an empty IpAddress object
     */
    IpAddress() {}

    /**
     * Construct an IpAddress object intialized with a string.
     *
     * @param str Human-readable string containing the IP Address in the form
     * of XXX.XXX.XXX.XXX
     */
    IpAddress( std::string str ) {
        m_str = str;
        inet_aton( m_str.c_str(), &m_addr );
    }

    /**
     * Set the IP Address
     *
     * @param str Human-readable string containing the IP Address in the form
     * of XXX.XXX.XXX.XXX
     */
    void SetIp( std::string str ) {
        if( inet_aton( str.c_str(), &m_addr ) != 1 ) {
            std::cerr << "Invalid address: " << str << std::endl;
            exit(1);
        }
        m_str = str;
    }

    /**
     * Getter for human-readable string.
     *
     * @returns The MAC address as a string.
     */
    std::string const & Str() const { return m_str; }

    /**
     * Getter for struct in_addr.
     *
     * @returns The MAC address as a struct in_addr.
     */
    struct in_addr const & Addr() const { return m_addr; }
};

/**
 * MacAddress class
 *
 * Storange class to holds a MAC address as both a human-readable string as 
 * well as an array of bytes.
 */
class MacAddress {

    std::string       m_str;
    std::vector<char> m_addr;

    public:

    /**
     * Constructs an empty MacAddress object.
     */
    MacAddress() { }

    /**
     * Constructs a MacAddress object initialized with the provided string.
     *
     * @param str String of the form xx:xx:xx:xx:xx:xx.
     */
    MacAddress( std::string const str ) {
        SetAddr(str);
    }

    /**
     * Set the stored MAC Address to the provided string.
     *
     * @param str String of the form xx:xx:xx:xx:xx:xx.
     */
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
    }

    /**
     * Getter for string representation of the stored MAC address.
     *
     * @returns String of the form xx:xx:xx:xx:xx
     */
    std::string const Str() const { return m_str; }

    /**
     * Getter for byte representation of the stored MAC address.
     *
     * @returns Vector of bytes.
     */
    std::vector<char> const Addr() const { return m_addr; }
};

/**
 * Prints a stream of bytes in a human-readable form.
 *
 * @param buf Pointer to the buffer.
 * @param size Number of bytes in the buffer.
 */
inline void print_bytes(unsigned char const * buf, const unsigned int size) {

    int it;

    printf( "Buffer =================================================\n" );
    printf( "Content:\n" );
    for(it = 0; it < size; it++) {
        unsigned char c = buf[it];
        printf( "%02X ", c );
        if( !((it + 1) %  4 ) ) printf("   ");
        if( !((it + 1) % 16 ) ) printf("\n");
    }
    printf("\n");
    printf( "========================================================\n" );
}

#endif /* _COMMON_HH_ */

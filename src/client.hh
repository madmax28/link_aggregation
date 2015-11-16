/** @file client.hh
 * Client class definition
 */

#ifndef _CLIENT_HH_
#define _CLIENT_HH_

#include <vector>

#include <netinet/ip.h>

#include "common.hh"
#include "nfqueue.hh"

/**
 * Client class
 *
 * The Client class handles communication with the client application. It
 * inherits from NfqHandler, which handles Netfilter packet mangling and
 * interception and reception.
 *
 * @see NfqHandler
 */
class Client : public NfqHandler {

    int m_socket;

    public:

    Client();
    ~Client();

    Buffer * RecvPkt();
    int SendPkt( Buffer const * buf ) const;

    /**
     * Getter for the file descriptor used by the NfqHandler parent.
     */
    int const RxFd() const { return NfqNlFd(); }
};

#endif /* _CLIENT_HH_ */

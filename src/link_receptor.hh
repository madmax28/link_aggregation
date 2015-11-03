#ifndef _LINK_RECEPTOR_HH_
#define _LINK_RECEPTOR_HH_

#include "piped_thread.hh"
#include "safe_queue.hh"
#include "link.hh"

#include <vector>
#include <string>

class LinkReceptor
        : public SafeQueue<AlaggPacket>
        , public PipedThread {

    // Stores reordered packets ready to be delivered to the client
    std::vector<Link *> m_links;

    static void recv(LinkReceptor *t) {
        std::cout << "LinkReceptor::recv()\n";
    }

    public:

    LinkReceptor(std::vector<std::string> peer_addresses,
                 std::vector<std::string> if_names) {
        // Initialize links
        for( int i = 0; i < peer_addresses.size(); i++ ) {
            m_links.push_back( new Link(if_names[i], peer_addresses[i]) );
        }

        // Start the reception thread
        SetThread(recv, this);
    }

    ~LinkReceptor() {
        for( int i = 0; i < m_links.size(); i++ ) {
            delete m_links[i];
        }
    }
};

#endif /* _LINK_RECEPTOR_HH_ */

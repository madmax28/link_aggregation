#ifndef _LINK_H_
#define _LINK_H_

class Link {

    /* Link properties */
    int m_port;
    int m_socket;
    bool m_is_open;

    public:

    Link(int port);
};

#endif /* _LINK_H_ */

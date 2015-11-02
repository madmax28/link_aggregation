#ifndef _PIPED_THREAD_HH_
#define _PIPED_THREAD_HH_

#include <unistd.h>
#include <thread>

#include "common.hh"

#define MSG_DONE "1"
#define MSG_LEN 1

/*
 * Thread class including communication with parent thread
 *   In addition to the regular std::thread capabilities, this class creates a
 *   pipe when the thread is created. After finishing f(), a MSG_DONE is written
 *   to the pipe.
 */

class PipedThread {

    struct Pipe {
        int m_rx;
        int m_tx;
    } m_pipe;

    std::thread m_thread;

    /*
     * Target function wrappers.
     *   Execute fun(), then notify other end of pipe.
     */

    template<typename F>
    void target(F fun) {

        // Call fun, the notify pipe
        fun();
        NotifyPipe();
    }

    template<typename F, typename A>
    void target(F fun, A args) {

        // Call fun, the notify pipe
        fun(args);
        NotifyPipe();
    }

    /*
     * Open the pipe and store it's fds
     */

    void OpenPipe() {

        int pfd[2];

        pipe(pfd);
        assert_perror(errno);

        m_pipe.m_rx = pfd[0];
        m_pipe.m_tx = pfd[1];
    }

    void NotifyPipe() const {
        write(m_pipe.m_tx, MSG_DONE, MSG_LEN);
        assert_perror(errno);
    }

    public:

    /*
     * Constructors
     */

    template<typename F>
    PipedThread(F fun)
            : m_thread(target<F>, fun) {
        OpenPipe();
    }

    template<typename F, typename A>
    PipedThread(F fun, A args)
            : m_thread(target<F, A>, fun, args) {
        OpenPipe();
    }

    void Join() {
        m_thread.join();
        // TODO
    }
};

#endif /* _PIPED_THREAD_HH_ */

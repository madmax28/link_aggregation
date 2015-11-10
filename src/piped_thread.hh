#ifndef _PIPED_THREAD_HH_
#define _PIPED_THREAD_HH_

// Pipe
#include <unistd.h>
#include <fcntl.h>

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

    public:

    enum piped_thread_except {
        except_pipefull = 0
    };

    // Execution mode
    enum piped_thread_mode {
        exec_repeat = -1,
        exec_once   = 1
    };

    private:

    // Number of thread executions, -1 = inf
    piped_thread_mode m_mode;
    std::thread       m_thread;

    struct Pipe {
        int m_rx;
        int m_tx;
    } m_pipe;

    /*
     * Target function wrappers.
     *   Execute fun(), then notify other end of pipe.
     */

    template<typename F>
    static void target(F fun, PipedThread *t) {

        do {
            // Call fun, then notify pipe
            fun();
//            if(fun())
//                t->NotifyPipe();
        } while(t->m_mode == exec_repeat);
    }

    template<typename F, typename A>
    static void target(F fun, A args, PipedThread *t) {

        do {
            // Call fun, the notify pipe
            fun(args);
//            if(fun(args))
//                t->NotifyPipe();
        } while(t->m_mode == exec_repeat);
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

        // Tx end should be non-blocking
        int fdflags = fcntl(m_pipe.m_tx, F_GETFL);
        assert_perror(errno);
        fcntl(m_pipe.m_tx, F_SETFL, fdflags | O_NONBLOCK);
        assert_perror(errno);
    }

    public:

    void NotifyPipe() const {
        write(m_pipe.m_tx, MSG_DONE, MSG_LEN);
        if((errno == EAGAIN) || (errno == EWOULDBLOCK))
            throw except_pipefull;
        else
            assert_perror(errno);
    }

    /*
     * Constructors
     */

    PipedThread() {
        OpenPipe();
    }

    template<typename F>
    PipedThread(F fun,
            piped_thread_mode mode = exec_once)
            : m_mode(mode)
            , m_thread(target<F>, fun, this) {
        OpenPipe();
    }

    template<typename F, typename A>
    PipedThread(F fun,
            A args,
            piped_thread_mode mode = exec_once)
            : m_mode(mode)
            , m_thread(target<F, A>, fun, args, this) {
        OpenPipe();
    }

    /*
     * Set thread function at a later point, in case empty constructor was
     * called
     */

    template<typename F>
    void SetThread(F fun, piped_thread_mode mode = exec_once) {
        m_mode = mode;
        m_thread = std::thread(target<F>, fun, this);
    }

    template<typename F, typename A>
    void SetThread(F fun, A args, piped_thread_mode mode = exec_once) {
        m_mode = mode;
        m_thread = std::thread(target<F, A>, fun, args, this);
    }

    // Wait for thread to finish
    void Join() {
        m_thread.join();
    }

    void * EmptyPipe() const {
        char c[MSG_LEN];
        int n = read(m_pipe.m_rx, c, MSG_LEN);
        assert(n == MSG_LEN);
    }

    int const PipeTxFd() const { return m_pipe.m_tx; }
    int const PipeRxFd() const { return m_pipe.m_rx; }
};

#endif /* _PIPED_THREAD_HH_ */

/** @file piped_thread.hh
 * PipedThread class definition
 */

#ifndef _PIPED_THREAD_HH_
#define _PIPED_THREAD_HH_

// Pipe
#include <unistd.h>
#include <fcntl.h>

#include <thread>

#include "common.hh"

/**
 * Trivial message definition used to notify pipes.
 */
#define MSG_DONE "1"
/**
 * Length of MSG_DONE
 */
#define MSG_LEN 1

/**
 * PipedThread class
 *
 * A class facilitating detached threads and including communication between
 * parent and child through a pipe.
 */
class PipedThread {

    public:

    /**
     * Enumeration of exceptions thrown by PipedThread.
     */
    enum piped_thread_except {
        // The pipe buffer is full
        except_pipefull = 0
    };

    /**
     * Modes of operation suppoed by PipedThread.
     *
     * Supported modes are single single and repeated thread execution.
     */
    enum piped_thread_mode {
        exec_repeat = -1,
        exec_once   = 1
    };

    private:

    // Number of thread executions, -1 = inf
    piped_thread_mode m_mode;
    std::thread       m_thread;

    /**
     * Structure of a communication pipe between parent and child process.
     */
    struct Pipe {
        int m_rx;
        int m_tx;
    } m_pipe;

    /**
     * Wrapper to call the callback function.
     *
     * @param fun Callback function
     * @param t Back-reference to calling PipedThread instance
     */
    template<typename F>
    static void target(F fun, PipedThread *t) {

        do {
            // Call fun, then notify pipe
            fun();
        } while(t->m_mode == exec_repeat);
    }

    /**
     * Wrapper to call the callback function with arguments.
     *
     * @param fun Callback function
     * @param args Callback function arguments
     * @param t Back-reference to calling PipedThread instance
     */
    template<typename F, typename A>
    static void target(F fun, A args, PipedThread *t) {

        do {
            // Call fun, the notify pipe
            fun(args);
        } while(t->m_mode == exec_repeat);
    }

    /**
     * Opens a pipe for communication. The transmission end of the pipe is made
     * non-blocking.
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

    /**
     * Send MSG_DONE to the associated pipe.
     */
    void NotifyPipe() const {

        write(m_pipe.m_tx, MSG_DONE, MSG_LEN);
        if((errno == EAGAIN) || (errno == EWOULDBLOCK))
            throw except_pipefull;
        else
            assert_perror(errno);
    }

    /**
     * PipedThread class constructor.
     *
     * Creates an empty PipedThread class. The communication pipe is opened,
     * however no thread is spawned.
     */
    PipedThread() {

        OpenPipe();
    }

    /**
     * PipedThread class constructor.
     *
     * Creates an empty PipedThread class. A new thread is spawned, and a
     * communcation pipe is opened.
     *
     * @param fun Function to be executed by the thread
     * @param mode Execution mode of the thread
     * @see PipedThread::piped_thread_mode
     */
    template<typename F>
    PipedThread(F fun,
            piped_thread_mode mode = exec_once)
            : m_mode(mode)
            , m_thread(target<F>, fun, this) {

        OpenPipe();
    }

    /**
     * PipedThread class constructor.
     *
     * Creates an empty PipedThread class. A new thread is spawned, and a
     * communcation pipe is opened.
     *
     * @param fun Function to be executed by the thread
     * @param args Arguments to be passed to the thread function
     * @param mode Execution mode of the thread
     * @see PipedThread::piped_thread_mode
     */
    template<typename F, typename A>
    PipedThread(F fun,

            A args,
            piped_thread_mode mode = exec_once)
            : m_mode(mode)
            , m_thread(target<F, A>, fun, args, this) {
        OpenPipe();
    }

    /**
     * Start the thread of an empty PipedThread object.
     *
     * @param fun Function to be executed by the thread
     * @param mode Execution mode of the thread
     * @see PipedThread::piped_thread_mode
     */
    template<typename F>
    void SetThread(F fun, piped_thread_mode mode = exec_once) {

        m_mode = mode;
        m_thread = std::thread(target<F>, fun, this);
    }

    /**
     * Start the thread of an empty PipedThread object.
     *
     * @param fun Function to be executed by the thread
     * @param args Arguments to be passed to the thread function
     * @param mode Execution mode of the thread
     * @see PipedThread::piped_thread_mode
     */
    template<typename F, typename A>
    void SetThread(F fun, A args, piped_thread_mode mode = exec_once) {
        m_mode = mode;
        m_thread = std::thread(target<F, A>, fun, args, this);
    }

    /**
     * Wait for the thread to finish.
     */
    void Join() {
        m_thread.join();
    }

    /**
     * Read one MSG_DONE from the pipe.
     */
    void * EmptyPipe() const {
        char c[MSG_LEN];
        int n = read(m_pipe.m_rx, c, MSG_LEN);
        assert(n == MSG_LEN);
    }

    /**
     * Getter for the communication pipe's transmission end file descriptor.
     *
     * @returns The pipes tx fd.
     */
    int const PipeTxFd() const { return m_pipe.m_tx; }

    /**
     * Getter for the communication pipe's receiver end file descriptor.
     *
     * @returns The pipes rx fd.
     */
    int const PipeRxFd() const { return m_pipe.m_rx; }
};

#endif /* _PIPED_THREAD_HH_ */

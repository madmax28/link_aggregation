/** @file timer.hh
 * Timer class definition
 */

#ifndef _TIMER_HH_
#define _TIMER_HH_

#include <cstdint>
#include <chrono>
#include <thread>

/**
 * Timer class.
 *
 * A simple class that is initialized with a callback function and it's
 * arguments, as well as a timeout.
 * A detached thread is spawned, sleeping for the specified time, then calling
 * the callback function.
 */
class Timer {

    std::thread m_t;

    /**
     * Sleep until the timeout occurs.
     *
     * @param msec Time in milliseconds to sleep for.
     * @param fun Callback function
     * @param args Callback function arguments
     */
    template<typename F, typename... A>
    static void t(uint32_t msec, F fun, A... args) {
        // Sleep
        std::this_thread::sleep_for(std::chrono::milliseconds(msec));
        // Callback
        fun(args...);
    }

    public:

    /**
     * Timer class constructor
     *
     * Spawns the timer thread and detaches it.
     *
     * @param to_msec Timeout in milliseconds.
     * @param fun Callback function
     * @param args Callback function arguments
     */
    template<typename F, typename... A>
    Timer(uint32_t to_msec, F fun, A... args)
          : m_t(std::thread(t<F, A...>, to_msec, fun, args...)) {

        m_t.detach();
    }
};

#endif /* _TIMER_HH_ */

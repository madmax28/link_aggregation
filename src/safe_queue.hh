/** @file safe_queue.hh
 * SafeQueue class definition
 */

#ifndef _SAFE_QUEUE_HH_
#define _SAFE_QUEUE_HH_

#include <queue>
#include <mutex>

/**
 * SafeQueue class
 *
 * A regular Queue class utilizing mutexes to facilitate multi-thread safety.
 */
template<typename T>
class SafeQueue {

    public:

    /**
     * Definitions of SafeQueue exceptions.
     */
    enum safe_queue_except {
        // Queue empty
        except_isempty = 0
    };

    // Underlying queue object
    std::queue<T> m_queue;
    std::mutex m_mutex;

    /**
     * SafeQueue class constructor.
     */
    SafeQueue() {}

    /**
     * Push an object to the queue.
     *
     * @param val Object to be pushed.
     */
    void Push(const T& val) {

        std::lock_guard<std::mutex> lock(m_mutex);

        m_queue.push(val);
    }

    /**
     * Pop an object from the queue.
     */
    void Pop() {

        std::lock_guard<std::mutex> lock(m_mutex);

        if(m_queue.empty())
            throw except_isempty;
        m_queue.pop();
    }

    /**
     * Access the front of the queue.
     *
     * @returns A reference to object at the front of the queue.
     */
    T& Front() {

        std::lock_guard<std::mutex> lock(m_mutex);

        if(m_queue.empty())
            throw except_isempty;
        return m_queue.front();
    }

    /**
     * Check if the queue is empty.
     *
     * @returns True if the queue is empty, False otherwise.
     */
    bool Empty() {

        std::lock_guard<std::mutex> lock(m_mutex);

        return m_queue.empty();
    }

    /**
     * Get the size of the queue.
     *
     * @returns The queue's size
     */
    size_t Size() {

        std::lock_guard<std::mutex> lock(m_mutex);

        return m_queue.size();
    }
};

#endif /* _SAFE_QUEUE_HH_ */

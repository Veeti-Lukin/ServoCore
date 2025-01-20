#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <cstddef>
#include <cstdint>

namespace utils {

/**
 * @brief Ring/Circular buffer works with a FIFO (First in First Out) principle
 *
 *        NOT INTERRUPT SAFE! if used from interrupts and "main thread" user is
 *        responsible for handling the interrupt safety
 *
 * @tparam size number of elements that ring buffer can fit. Max value is size_t MAX -1.!!
 */
template <size_t size>
class RingBuffer {
public:
     RingBuffer() = default;
    ~RingBuffer() = default;

    /**
     * @brief Adds a new item into the ring buffer
     * @param item the item to but into the buffer.
     * @return true if the element fitted in to ring buffer, false otherwise
     */
    bool push(uint8_t item) volatile;

    /**
     * @brief Gets and removes the oldest element from the ring buffer
     * @return the oldest element, or 0 if the ring buffer is empty
     */
    uint8_t pop() volatile;

    /**
     * @brief Only gets the oldest element from the ring buffer
     *        does not remove it from the buffer
     * @return the oldest element, or 0 if the ring buffer is empty
     */
    [[nodiscard]] uint8_t peek() volatile const;

    /**
     * @brief Checks if the ring buffer is full
     * @return true if full, false otherwise
     */
    [[nodiscard]] bool isFull() volatile const;

    /**
     * @brief Checks if the ring buffer is empty
     * @return true if empty, false otherwise
     */
    [[nodiscard]] bool isEmpty() volatile const;

    /**
     * @brief Tells the number of bytes available for reading in the buffer
     * @return the number of bytes available for reading
     */
    [[nodiscard]] size_t bytesAvailable() const volatile;

private:
    uint8_t buffer_[size] = {0};
    size_t  head_         = 0;
    size_t  tail_         = 0;
};

/// ------------------------ DEFINITIONS --------------------------------------

template <size_t size>
bool RingBuffer<size>::isEmpty() const volatile {
    return tail_ == head_;
}

template <size_t size>
bool RingBuffer<size>::isFull() const volatile {
    // Potential overflow if the size happens to be max value of size_t
    size_t index_after_head = head_ + 1;
    // wrap around in the end
    if (index_after_head == size) {
        index_after_head = 0;
    }

    return index_after_head == tail_;
}

template <size_t size>
uint8_t RingBuffer<size>::peek() const volatile {
    if (isEmpty()) return 0;

    return buffer_[tail_];
}

template <size_t size>
uint8_t RingBuffer<size>::pop() volatile {
    if (isEmpty()) return 0;

    const uint8_t element = buffer_[tail_];

    tail_++;
    // wrap around in the end
    if (tail_ == size) {
        tail_ = 0;
    }

    return element;
}

template <size_t size>
bool RingBuffer<size>::push(uint8_t item) volatile {
    if (isFull()) return false;

    buffer_[head_] = item;
    head_++;

    // wrap around in the end
    if (head_ == size) {
        head_ = 0;
    }

    return true;
}

template <size_t size>
size_t RingBuffer<size>::bytesAvailable() const volatile {
    if (head_ >= tail_) {
        return head_ - tail_;
    }

    return size - tail_ + head_;
}

}  // namespace utils

#endif  // ROBOBOT_DRIVER_BOARD_RINGBUFFER_H

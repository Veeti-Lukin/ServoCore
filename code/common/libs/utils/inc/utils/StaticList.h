#ifndef COMMON_LIBS_UTILS_STATICLIST_H
#define COMMON_LIBS_UTILS_STATICLIST_H

#include <initializer_list>
#include <memory>
#include <utility>

#include "assert/assert.h"

namespace utils {

/**
 *  @brief A fixed-capacity list container that stores elements in contiguous storage.
 *
 *        Offers iterator style access for range based loops
 *
 * @tparam T Type of the elements stored in the list.
 * @tparam N Maximum capacity of the list.
 */
template <typename T, size_t N>
class StaticList {
public:
                StaticList()                  = default;
    ~           StaticList()                  = default;
                StaticList(const StaticList&) = default;
                StaticList(StaticList&&)      = default;
    StaticList& operator=(const StaticList&)  = default;
    StaticList& operator=(StaticList&&)       = default;

    StaticList(std::initializer_list<T> init);
    StaticList(std::span<T> init);

    /* ######################## Element access ######################## */
    /**
     * @brief Accesses the element at the specified index.
     * @param index Index of the element to access.
     * @return Reference to the element at the specified index.
     */
    [[nodiscard]] T& operator[](size_t index);
    /**
     * @brief Accesses the element at the specified index (const).
     * @param index Index of the element to access.
     * @return Const reference to the element at the specified index.
     */
    [[nodiscard]] const T& operator[](size_t index) const;
    /**
     * @brief Accesses the first element.
     * @return Reference to the first element.
     */
    [[nodiscard]] T& front();
    /**
     * @brief Accesses the first element (const).
     * @return Const reference to the first element.
     */
    [[nodiscard]] const T& front() const;
    /**
     * @brief Accesses the last element.
     * @return Reference to the last element.
     */
    [[nodiscard]] T& back();
    /**
     * @brief Accesses the last element (const).
     * @return Const reference to the last element.
     */
    [[nodiscard]] const T& back() const;
    /**
     * @brief Returns a pointer to the underlying array.
     * @return Pointer to the internal data array.
     */
    [[nodiscard]] T* data();

    /* ######################## Capacity info ######################## */
    /**
     * @brief Checks if the list is empty.
     * @return True if empty, false otherwise.
     */
    [[nodiscard]] bool empty() const;
    /**
     * @brief Checks if the list is full.
     * @return True if full, false otherwise.
     */
    [[nodiscard]] bool full() const;
    /**
     * @brief Gets the current number of elements in the list.
     * @return Current size of the list.
     */
    [[nodiscard]] size_t size() const;
    /**
     * @brief Gets the maximum capacity of the list.
     * @return Maximum number of elements the list can hold.
     */
    [[nodiscard]] size_t capacity() const;

    /* ######################## Modifiers ######################## */
    /**
     * @brief Adds an element to the end of the list.
     * @param value Value to add.
     */
    void pushBack(const T& value);
    /**
     * @brief Inserts an element at the front of the list, shifting all existing elements.
     * @param value Value to insert at the front.
     */
    void pushFront(const T& value);
    /**
     * @brief Removes and returns the last element.
     * @return The removed element.
     */
    T popBack();
    /**
     * @brief Removes and returns the first element.
     * @return The removed element.
     */
    T popFront();
    /**
     * @brief Removes an element at the specified index.
     * @param index Index of the element to remove.
     */
    void removeElement(size_t index);
    /**
     * @brief Removes an element using its pointer.
     * @param iterator Pointer to the element to remove.
     */
    void removeElement(T* iterator);
    /**
     * @brief Remove all the elements from the list and set size to 0
     */
    void clear();
    /**
     * @brief Constructs an element in-place at the end of the list.
     * @tparam Args Argument types for element construction.
     * @param args Arguments to forward to the element constructor.
     * @return Reference to the newly emplaced element.
     */
    template <typename... Args>
    T& emplaceBack(Args&&... args);

    // Iterators
    /**
     * @brief Gets a pointer to the first element. Used for range based loops.
     * @return Pointer to the first element.
     */
    T* begin();
    /**
     * @brief Gets a pointer to one past the last element. Used for range based loops.
     * @return Pointer to one past the end of the list.
     */
    T* end();
    /**
     * @brief Gets a const pointer to the first element. Used for range based loops.
     * @return Const pointer to the first element.
     */
    const T* begin() const;
    /**
     * @brief Gets a const pointer to one past the last element. Used for range based loops.
     * @return Const pointer to one past the end of the list.
     */
    const T* end() const;

private:
    T      data_[N];
    size_t size_ = 0;
};

//
//
//
//
//
//

/// ------------------------ DEFINITIONS --------------------------------------

template <typename T, size_t N>
StaticList<T, N>::StaticList(std::initializer_list<T> init) {
    ASSERT(init.size() <= N);
    size_ = init.size();
    std::copy(init.begin(), init.end(), data_);
}

template <typename T, size_t N>
StaticList<T, N>::StaticList(std::span<T> init) {
    ASSERT(init.size() <= N);
    size_ = init.size();
    std::copy(init.begin(), init.end(), data_);
}

template <typename T, size_t N>
T& StaticList<T, N>::operator[](size_t index) {
    ASSERT(index < size_);
    return data_[index];
}

template <typename T, size_t N>
const T& StaticList<T, N>::operator[](size_t index) const {
    ASSERT(index < size_);
    return data_[index];
}

template <typename T, size_t N>
T& StaticList<T, N>::front() {
    ASSERT(size_ > 0);
    return data_[0];
}

template <typename T, size_t N>
const T& StaticList<T, N>::front() const {
    ASSERT(size_ > 0);
    return data_[0];
}

template <typename T, size_t N>
T& StaticList<T, N>::back() {
    ASSERT(size_ > 0);
    return data_[size_ - 1];
}

template <typename T, size_t N>
const T& StaticList<T, N>::back() const {
    ASSERT(size_ > 0);
    return data_[size_ - 1];
}

template <typename T, size_t N>
T* StaticList<T, N>::data() {
    return data_;
}

template <typename T, size_t N>
bool StaticList<T, N>::empty() const {
    return size_ == 0;
}

template <typename T, size_t N>
bool StaticList<T, N>::full() const {
    return size_ == N;
}

template <typename T, size_t N>
size_t StaticList<T, N>::size() const {
    return size_;
}

template <typename T, size_t N>
size_t StaticList<T, N>::capacity() const {
    return N;
}

template <typename T, size_t N>
void StaticList<T, N>::pushBack(const T& value) {
    ASSERT(size_ < N);
    data_[size_] = value;
    size_++;
}

template <typename T, size_t N>
void StaticList<T, N>::pushFront(const T& value) {
    ASSERT(size_ < N);
    for (size_t i = size_; i > 0; --i) {
        data_[i] = data_[i - 1];
    }
    data_[0] = value;
    size_++;
}

template <typename T, size_t N>
T StaticList<T, N>::popBack() {
    ASSERT(size_ > 0);
    T result = std::move(data_[size_ - 1]);
    // Explicitly call destructor. (NO-OP for trivial types)
    data_[size_ - 1].~T();
    size_--;
    return result;
}

template <typename T, size_t N>
T StaticList<T, N>::popFront() {
    ASSERT(size_ > 0);
    T result = std::move(data_[0]);
    // Explicitly call destructor. (NO-OP for trivial types)
    data_[0].~T();

    // Shift all other elements one index smaller position
    for (size_t i = 1; i < size_; i++) {
        data_[i - 1] = data_[i];
    }
    size_--;
    return result;
}

template <typename T, size_t N>
void StaticList<T, N>::removeElement(size_t index) {
    ASSERT(index < size_);
    // Explicitly call destructor. (NO-OP for trivial types)
    data_[0].~T();
    for (size_t i = index + 1; i < size_; ++i) {
        data_[i - 1] = data_[i];
    }
    size_--;
}

template <typename T, size_t N>
void StaticList<T, N>::removeElement(T* iterator) {
    ASSERT(iterator >= data_ && iterator < data_ + size_);
    size_t index = static_cast<size_t>(iterator - data_);
    removeElement(index);
}

template <typename T, size_t N>
void StaticList<T, N>::clear() {
    for (size_t i = 0; i < size_; i++) {
        // Explicitly call destructor. (NO-OP for trivial types)
        data_[i].~T();
    }
    size_ = 0;
}

template <typename T, size_t N>
template <typename... Args>
T& StaticList<T, N>::emplaceBack(Args&&... args) {
    data_[size_] = std::move(T(std::forward<Args>(args)...));
    return data_[size_++];
}

template <typename T, size_t N>
T* StaticList<T, N>::begin() {
    return data_;
}

template <typename T, size_t N>
T* StaticList<T, N>::end() {
    return data_ + size_;
}

template <typename T, size_t N>
const T* StaticList<T, N>::begin() const {
    return data_;
}

template <typename T, size_t N>
const T* StaticList<T, N>::end() const {
    return data_ + size_;
}

}  // namespace utils

#endif  // COMMON_LIBS_UTILS_STATICLIST_H

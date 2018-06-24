//
// Created by matveich on 23.06.18.
//

#ifndef BIGINT_MY_SHARED_PTR_H
#define BIGINT_MY_SHARED_PTR_H

#include <cassert>

template<typename T>
class my_shared_ptr {
public:
    my_shared_ptr() : data(nullptr),
                      capacity(nullptr),
                      counter(nullptr) {}

    my_shared_ptr(T *p, size_t _capacity) : data(p),
                                            capacity(new size_t(_capacity)),
                                            counter(new size_t(1)) {}

    my_shared_ptr(const my_shared_ptr &other) : data(other.data),
                                                capacity(other.capacity),
                                                counter(other.counter) {
        ++*counter;
    }

    ~my_shared_ptr() {
        reset();
    }

    T *get() {
        return data;
    }

    void reset() {
        if (--*counter == 0)
            release();
        counter = nullptr;
        data = nullptr;
        capacity = nullptr;
    }

    void reset(T *new_data, size_t _capacity) {
        reset();
        data = new_data;
        capacity = new size_t(_capacity);
        counter = new size_t(1);
    }

    size_t get_capacity() {
        return *capacity;
    }

    my_shared_ptr &operator=(my_shared_ptr other) {
        swap(other);
        return *this;
    }

    void swap(my_shared_ptr &other) {
        std::swap(data, other.data);
        std::swap(capacity, other.capacity);
        std::swap(counter, other.counter);
    }

    bool unique() {
        return *counter == 1;
    }

    void release() {
        delete capacity;
        delete[] data;
        delete counter;
    }

private:
    T *data;
    size_t *capacity;
    size_t *counter;
};


#endif //BIGINT_MY_SHARED_PTR_H

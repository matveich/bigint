//
// Created by matveich on 23.06.18.
//

#ifndef BIGINT_MY_SHARED_PTR_H
#define BIGINT_MY_SHARED_PTR_H

#include <cassert>

template<typename T>
class my_shared_ptr {
public:
    my_shared_ptr() : data(nullptr) {}

    my_shared_ptr(T *p, size_t data_size) : data(p) {
        data[0] = 1;
        data[1] = data_size;
    }

    my_shared_ptr(const my_shared_ptr &other) : data(other.data) {
        if (data)
            ++data[0];
    }

    ~my_shared_ptr() {
        reset();
    }

    T *get() const {
        return data + 2;
    }

    void reset() {
        if (data && --data[0] == 0)
            release();
        data = nullptr;
    }

    void reset(T *new_data, size_t data_size) {
        reset();
        data = new_data;
        data[0] = 1;
        data[1] = data_size;
    }

    size_t get_counter() const {
        return data[0];
    }

    size_t get_capacity() const {
        return data[1];
    }

    bool unique() const {
        return get_counter() == 1;
    }

    my_shared_ptr &operator=(my_shared_ptr other) {
        std::swap(data, other.data);
        return *this;
    }

private:
    // syntax: 0-byte - counter, 1-byte - capacity, then - data
    T *data;

    void release() {
        delete[] data;
    }
};

#endif //BIGINT_MY_SHARED_PTR_H

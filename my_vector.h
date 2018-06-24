//
// Created by matveich on 19.06.18.
//

#ifndef BIGINT_MY_VECTOR_H
#define BIGINT_MY_VECTOR_H

#include <iostream>
#include "my_shared_ptr.h"

class my_vector {
public:
    typedef unsigned int ui;

    my_vector();

    my_vector(size_t size);

    my_vector(size_t size, ui val);

    my_vector(const std::initializer_list<ui> &list);

    ~my_vector();

    my_vector(const my_vector &other) noexcept;

    void reserve(size_t capacity);

    void resize(size_t size);

    void resize(size_t size, ui val);

    void push_back(ui x);

    void pop_back();

    size_t get_capacity();

    size_t size() const;

    void clear();

    const ui &operator[](size_t i) const;

    ui &operator[](size_t i);

    ui back() const;

    my_vector &operator=(const my_vector &other) noexcept;

    ui *get_data() const;

    ui* copy_and_get();

private:
    static const size_t SMALL_DATA_SIZE = 4;
    static const size_t ENLARGE_X = 2;
    bool is_big;
    size_t _size;
    ui *real_data;


    union abstract_data {
        ui small[SMALL_DATA_SIZE]{};
        my_shared_ptr<ui> big;

        abstract_data() {}

        ~abstract_data() {}
    } data;
};


#endif //BIGINT_MY_VECTOR_H

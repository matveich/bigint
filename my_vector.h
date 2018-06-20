//
// Created by matveich on 19.06.18.
//

#ifndef BIGINT_MY_VECTOR_H
#define BIGINT_MY_VECTOR_H

#include <iostream>
#include <memory>
#include <utility>

class my_vector {
public:
    typedef unsigned int ui;

    my_vector();

    my_vector(size_t size);

    my_vector(size_t size, ui val);

    my_vector(const std::initializer_list<ui> &list);

    ~my_vector();

    my_vector(const my_vector &other) noexcept ;

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

    void insert_to_begin(size_t amount, ui val);

    ui* get_data();
    ui* get_data() const;
    ui* get_data_and_check();

private:
    static const size_t SMALL_DATA_SIZE = 4;
    static const size_t ENLARGE_X = 2;
    bool is_big;
    size_t _size;
    ui *real_data;

    struct big_storage {
        std::shared_ptr<ui> ptr;
        size_t _capacity;

        big_storage() noexcept :
                ptr(nullptr, std::default_delete<ui[]>()),
                _capacity(0) {}

        big_storage(ui *p, size_t capacity) noexcept :
                ptr(p, std::default_delete<ui[]>()),
                _capacity(capacity) {}

        big_storage(const big_storage &other) noexcept = default;
    };

    union abstract_data {
        ui small[SMALL_DATA_SIZE]{};
        big_storage big;

        abstract_data() {}

        ~abstract_data() {}
    } data;

    void data_check();
};


#endif //BIGINT_MY_VECTOR_H

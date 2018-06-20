//
// Created by matveich on 19.06.18.
//

#include "my_vector.h"


my_vector::my_vector() : is_big(false),
                         _size(0),
                         real_data(data.small) {}

my_vector::my_vector(size_t size): my_vector(size, 0) {
}

my_vector::~my_vector() {
    if (is_big)
        data.big.~big_storage();
}

my_vector::my_vector(size_t size, my_vector::ui val): my_vector() {
    reserve(size);
    std::fill(real_data, real_data + size, val);
    _size = size;
}

my_vector::my_vector(const my_vector &other) noexcept: my_vector() {
    is_big = other.is_big;
    _size = other._size;
    if (other.is_big) {
        new(&data.big)big_storage(other.data.big);
        real_data = data.big.ptr.get();
    } else {
        std::copy(other.data.small, other.data.small + other._size, data.small);
        real_data = data.small;
    }
}

my_vector::my_vector(const std::initializer_list<my_vector::ui> &list): my_vector() {
    reserve(list.size());
    _size = list.size();
    std::copy(list.begin(), list.end(), real_data);
}

void my_vector::reserve(size_t _capacity) {
    if (_capacity < get_capacity() - 1)
        return;
    _capacity = std::max(_capacity, get_capacity() * ENLARGE_X);
    if (is_big) {
        auto p = data.big.ptr;
        data.big.ptr.reset(new ui[_capacity], std::default_delete<ui[]>());
        std::copy(p.get(), p.get() + _size, data.big.ptr.get());
    } else {
        is_big = true;
        auto new_data = new ui[_capacity];
        std::copy(real_data, real_data + _size, new_data);
        new(&data.big)big_storage(new_data, _capacity);
    }
    real_data = data.big.ptr.get();
    data.big._capacity = _capacity;
}

void my_vector::resize(size_t size, my_vector::ui val) {
    reserve(size);
    if (size > _size)
        std::fill(real_data + _size, real_data + size, val);
    _size = size;
}

void my_vector::resize(size_t size) {
    resize(size, 0);
}

size_t my_vector::get_capacity() {
    return is_big ? data.big._capacity : SMALL_DATA_SIZE;
}

void my_vector::push_back(my_vector::ui x) {
    data_check();
    if (get_capacity() < _size + 1)
        reserve(get_capacity() * ENLARGE_X);
    real_data[_size++] = x;
}

void my_vector::pop_back() {
    _size--;
}

const my_vector::ui &my_vector::operator[](size_t i) const {
    return real_data[i];
}

my_vector::ui &my_vector::operator[](size_t i) {
    data_check();
    return real_data[i];
}

void my_vector::data_check() {
    if (is_big && !data.big.ptr.unique()) {
        data.big.ptr.reset(new ui[std::min(data.big._capacity, _size * ENLARGE_X)], std::default_delete<ui[]>());
        std::copy(real_data, real_data + _size, data.big.ptr.get());
        real_data = data.big.ptr.get();
    }
}

my_vector &my_vector::operator=(const my_vector &other) noexcept {
    _size = other._size;
    if (other.is_big) {
        if (is_big)
            data.big = big_storage(other.data.big);
        else
            new(&data.big)big_storage(other.data.big);
        real_data = data.big.ptr.get();
    }
    else {
        if (is_big)
            data.big.~big_storage();
        std::copy(other.data.small, other.data.small + other._size, data.small);
        real_data = const_cast<ui*>(data.small);
    }
    is_big = other.is_big;
    return *this;
}

size_t my_vector::size() const {
    return _size;
}

void my_vector::clear() {
    _size = 0;
}

my_vector::ui my_vector::back() const {
    return real_data[_size - 1];
}

void my_vector::insert_to_begin(size_t amount, my_vector::ui val) {
    size_t new_cap = std::max(get_capacity() * ENLARGE_X, _size + amount);
    auto cp = new ui[new_cap];
    std::copy(real_data, real_data + _size, cp + amount);
    std::fill(cp, cp + amount, val);
    if (new_cap > SMALL_DATA_SIZE) {
        if (is_big) {
            data.big._capacity = new_cap;
            data.big.ptr.reset(cp, std::default_delete<ui[]>());
        }
        else {
            is_big = true;
            new(&data.big)big_storage(cp, new_cap);
        }
        real_data = data.big.ptr.get();
    }
    else
        std::copy(cp, cp + SMALL_DATA_SIZE, data.small);
    _size += amount;
}

my_vector::ui* my_vector::get_data() {
    return real_data;
}

my_vector::ui* my_vector::get_data() const {
    return real_data;
}

my_vector::ui* my_vector::get_data_and_check() {
    data_check();
    return real_data;
}
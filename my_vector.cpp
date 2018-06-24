//
// Created by matveich on 19.06.18.
//

#include "my_vector.h"


my_vector::my_vector() : is_big(false),
                         _size(0),
                         real_data(data.small) {}

my_vector::my_vector(size_t size) : my_vector(size, 0) {}

my_vector::~my_vector() {
    if (is_big) {
        is_big = false;
        data.big.reset();
    }
}

my_vector::my_vector(size_t size, my_vector::ui val) : my_vector() {
    reserve(size);
    std::fill(real_data, real_data + size, val);
    _size = size;
}

my_vector::my_vector(const my_vector &other) noexcept: my_vector() {
    is_big = other.is_big;
    _size = other._size;
    if (other.is_big) {
        new(&data.big)my_shared_ptr<ui>(other.data.big);
        real_data = data.big.get();
    } else {
        std::copy(other.data.small, other.data.small + other._size, data.small);
        real_data = data.small;
    }
}

my_vector::my_vector(const std::initializer_list<my_vector::ui> &list) : my_vector() {
    reserve(list.size());
    _size = list.size();
    std::copy(list.begin(), list.end(), real_data);
}

void my_vector::reserve(size_t _capacity) {
    if (_capacity < get_capacity() - 1)
        return;
    _capacity = std::max(_capacity, get_capacity() * ENLARGE_X);
    if (is_big) {
        auto p = data.big;
        data.big.reset(new ui[_capacity + 2], _capacity);
        std::copy(p.get(), p.get() + _size, data.big.get());
    } else {
        is_big = true;
        auto new_data = new ui[_capacity + 2];
        std::copy(real_data, real_data + _size, new_data + 2);
        new(&data.big)my_shared_ptr<ui>(new_data, _capacity);
    }
    real_data = data.big.get();
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
    return is_big ? data.big.get_capacity() : SMALL_DATA_SIZE;
}

void my_vector::push_back(my_vector::ui x) {
    copy_and_get();
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
    copy_and_get();
    return real_data[i];
}

my_vector::ui* my_vector::copy_and_get() {
    if (!is_big || data.big.unique())
        return real_data;
    auto new_cap = std::min(data.big.get_capacity(), _size * ENLARGE_X);
    data.big.reset(new ui[new_cap + 2], new_cap);
    std::copy(real_data, real_data + _size, data.big.get());
    return real_data = data.big.get();
}

my_vector &my_vector::operator=(const my_vector &other) noexcept {
    _size = other._size;
    if (other.is_big) {
        if (is_big)
            data.big = other.data.big;
        else
            new(&data.big)my_shared_ptr<ui>(other.data.big);
        real_data = data.big.get();
    } else {
        if (is_big)
            data.big.reset();
        std::copy(other.data.small, other.data.small + other._size, data.small);
        real_data = data.small;
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

my_vector::ui *my_vector::get_data() const {
    return real_data;
}

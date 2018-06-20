//
// Created by matveich on 30.03.18.
//

#include "big_integer.h"

const big_integer::ui BASE = 32;
const big_integer::ui MAX_INT = UINT32_MAX;
const big_integer::ull SHIFT_BASE = ((big_integer::ull) 1 << BASE);
const big_integer::ui BASE_10 = 1000000000;

size_t big_integer::length() const {
    return number.size();
}

template<typename T>
big_integer::ui big_integer::cast_to_ui(T x) const {
    return static_cast<ui>(x & MAX_INT);
}

big_integer::big_integer() :
        sign(false),
        number({0}) {}

big_integer::big_integer(int source) :
        sign(source < 0),
        number({cast_to_ui(source)}) {}

big_integer::big_integer(big_integer const &source) :
        sign(source.sign),
        number(source.number) {
    shrink();
}

big_integer::big_integer(std::string source) :
        sign(false),
        number({0}) {
    bool _sign = (source[0] == '-');
    size_t it = (_sign ? 1 : 0);
    for (; it + 9 <= source.length(); it += 9)
        *this = *this * BASE_10 + stoi(source.substr(it, 9));
    if (it < source.length())
        *this = *this * static_cast<ui>(pow(10, source.length() - it))
                + stoi(source.substr(it));
    if (_sign)
        *this = -(*this);
    shrink();
}

big_integer::~big_integer() {
    sign = false;
    number = my_vector();
}

big_integer &big_integer::operator=(big_integer const &other) {
    sign = other.sign;
    number = other.number;
    return *this;
}

big_integer big_integer::operator+() const {
    return *this;
}

big_integer big_integer::operator-() const {
    if (*this == 0)
        return *this;
    big_integer res;
    auto _size = number.size();
    res.number.resize(_size + 1);
    auto data = number.get_data();
    auto res_data = res.number.get_data_and_check();
    for (size_t i = 0; i < _size; ++i)
        res_data[i] = ~data[i];
    res.shrink();
    res.add_one();
    res.sign = !sign;
    return res;
}

void big_integer::add_one() {
    bool cf = true;
    auto data = number.get_data_and_check();
    auto _size = number.size();
    for (size_t i = 0; cf && i < _size; ++i) {
        data[i] += 1;
        cf = data[i] == 0;
    }
    if (cf)
        number.push_back(1);
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
    ull cf = 0;
    big_integer result = rhs;
    size_t n = std::max(number.size(), result.number.size()) + 2;
    int eq_len = number.size() < result.number.size() ? -1 :
                 number.size() == result.number.size() ? 0 :
                 1;
    size_t h_i = number.size() - 1;
    number.resize(n, sign ? MAX_INT : 0);
    result.number.resize(n, result.sign ? MAX_INT : 0);
    auto data = number.get_data_and_check();
    auto res_data = result.number.get_data_and_check();
    for (size_t it = 0; it < n; ++it) {
        cf = cf + data[it] + res_data[it];
        res_data[it] = cast_to_ui(cf);
        cf >>= BASE;
    }

    result.sign = sign;
    if (sign != rhs.sign) {
        if (eq_len == -1)
            result.sign = !sign;
        else if (eq_len == 0) {
            if ((!sign && res_data[h_i] > data[h_i]) || (sign && res_data[h_i] < data[h_i]))
                result.sign = !sign;
        }
    }
    result.shrink();
    return *this = result;
}

big_integer &big_integer::operator-=(big_integer const &rhs) {
    return *this += -rhs;
}

big_integer operator+(big_integer a, big_integer const &b) {
    return a += b;
}

big_integer operator-(big_integer a, big_integer const &b) {
    return a -= b;
}

big_integer operator*(big_integer a, big_integer const &b) {
    return a *= b;
}

big_integer operator/(big_integer a, big_integer const &b) {
    return a /= b;
}

big_integer operator%(big_integer a, big_integer const &b) {
    return a %= b;
}

big_integer big_integer::mul_long_short(big_integer::ui x, size_t offset) const {
    big_integer result;
    result.sign = sign;
    size_t n = length() + offset + 2;
    result.number.resize(n, sign ? UINT32_MAX : 0);
    auto res_data = result.number.get_data();
    auto data = number.get_data();
    ull mul = 0, cf = 0;
    for (size_t i = offset; i < n - 2; i++) {
        mul = static_cast<ull>(data[i - offset]) * x + cf;
        res_data[i] = cast_to_ui(mul);
        cf = mul >> BASE;
    }
    for (size_t i = n - 2; i < n; i++) {
        mul = static_cast<ull>(sign ? MAX_INT : 0) * x + cf;
        res_data[i] = cast_to_ui(mul);
        cf = mul >> BASE;
    }
    result.shrink();
    return result;
}

big_integer big_integer::quotient(big_integer::ui x) const {
    big_integer result;
    result.sign = sign;
    size_t n = length();
    result.number.resize(n, sign ? UINT32_MAX : 0);
    ull div = 0, cf = 0;
    for (int i = static_cast<int>(n - 1); i >= 0; i--) {
        div = static_cast<ull>((*this)[i]) + (cf << BASE);
        result.number[i] = cast_to_ui(div / x);
        cf = div % x;
    }
    result.shrink();
    return result;
}

big_integer::ui big_integer::remainder(big_integer::ui x) const {
    size_t n = length();
    ull cf = 0;
    for (int i = static_cast<int>(n - 1); i >= 0; i--)
        cf = ((cf << BASE) + (*this)[i]) % x;
    return cast_to_ui(cf);
}

std::pair<big_integer, big_integer> big_integer::div_mod(big_integer const &rhs) {
    if (rhs == 0)
        throw "division by zero";
    if (rhs.length() > length())
        return std::make_pair(0, *this);
    if (rhs.length() == 1)
        return std::make_pair(quotient(rhs[0]), remainder(rhs[0]));
    big_integer b(rhs);

    //auto data = number.get_data_and_check();
    b.number.get_data_and_check();

    // normalize b
    ui k = static_cast<ui>(ceil(log2((static_cast<double>(cast_to_ui(SHIFT_BASE >> 1)) / b.number.back()))));
    b <<= k;
    (*this) <<= k;
    size_t m = length() - b.length();
    size_t n = b.length();
    big_integer q;
    q.number.resize(m + 1);
    auto q_data = q.number.get_data_and_check();
    b <<= (BASE * m);
    if (*this > b) {
        q_data[m] = 1;
        *this -= b;
    }

    for (int j = static_cast<int>(m - 1); j >= 0; j--) {
        q_data[j] = cast_to_ui(((static_cast<ull>((*this)[n + j]) << BASE) + (*this)[n + j - 1]) / b.number.back());
        b >>= BASE;
        *this -= b.mul_long_short(q_data[j], 0);

        // is executed twice in the worst case
        while (sign) {
            q_data[j]--;
            *this += b;
        }
    }
    q.shrink();
    shrink();
    return std::make_pair(q, *this >> k);
}

big_integer &big_integer::operator*=(big_integer const &rhs) {
    big_integer storage = *this, rhs_copy = rhs;
    bool s1 = sign, s2 = rhs_copy.sign;
    if (s1) storage = -storage;
    if (s2) rhs_copy = -rhs_copy;
    *this = 0;
    storage.number.get_data_and_check();
    auto rhs_data = rhs_copy.number.get_data_and_check();
    for (size_t i = 0; i < rhs_copy.length(); i++)
        *this += storage.mul_long_short(rhs_data[i], i);
    shrink();
    if (s1 != s2) *this = -(*this);
    return *this;
}

big_integer &big_integer::operator/=(big_integer const &rhs) {
    bool s1 = sign, s2 = rhs.sign;
    if (s1) *this = -(*this);
    *this = !s2 ? div_mod(rhs).first : div_mod(-rhs).first;
    if (s1 != s2) *this = -(*this);
    return *this;
}

big_integer &big_integer::operator%=(big_integer const &rhs) {
    bool s1 = sign, s2 = rhs.sign;
    if (s1) *this = -(*this);
    *this = !s2 ? div_mod(rhs).second : div_mod(-rhs).second;
    if (s1) *this = -(*this);
    return *this;
}

bool operator==(big_integer const &a, big_integer const &b) {
    if (a.sign != b.sign || a.length() != b.length())
        return false;
    for (int i = static_cast<int>(a.length()) - 1; i >= 0; i--) {
        if (a.number[i] != b.number[i])
            return false;
    }
    return true;
}

bool operator!=(big_integer const &a, big_integer const &b) {
    return !(a == b);
}

bool operator<(big_integer const &a, big_integer const &b) {
    if (a.sign != b.sign)
        return a.sign > b.sign;
    if (a.length() != b.length())
        return a.sign ^ (a.length() < b.length());
    auto a_data = a.number.get_data();
    auto b_data = b.number.get_data();
    for (int i = static_cast<int>(a.length()) - 1; i >= 0; i--) {
        if (a_data[i] != b_data[i])
            return a.sign ^ (a_data[i] < b_data[i]);
    }
    return false;
}

bool operator>(big_integer const &a, big_integer const &b) {
    return !(a < b || a == b);
}

bool operator<=(big_integer const &a, big_integer const &b) {
    return a < b || a == b;
}

bool operator>=(big_integer const &a, big_integer const &b) {
    return a > b || a == b;
}

big_integer big_integer::operator~() const {
    return -(*this) - 1;
}

template<class FunctorT>
big_integer &big_integer::apply_bitwise_operation(big_integer const &rhs, FunctorT functor) {
    auto data = number.get_data_and_check();
    for (size_t i = 0; i < length(); i++)
        data[i] = functor(data[i], rhs.number[i]);
    sign = functor(sign, rhs.sign);
    return *this;
}

big_integer &big_integer::operator&=(big_integer const &rhs) {
    return apply_bitwise_operation(rhs, std::bit_and<ui>());
}

big_integer operator&(big_integer a, big_integer const &b) {
    return a &= b;
}


big_integer &big_integer::operator|=(big_integer const &rhs) {
    return apply_bitwise_operation(rhs, std::bit_or<ui>());
}

big_integer operator|(big_integer a, big_integer const &b) {
    return a |= b;
}

big_integer &big_integer::operator^=(big_integer const &rhs) {
    return apply_bitwise_operation(rhs, std::bit_xor<ui>());
}

big_integer operator^(big_integer a, big_integer const &b) {
    return a ^= b;
}

big_integer &big_integer::operator<<=(int rhs) {
    if (rhs < 0)
        throw "error! right value is negative";
    bool negative_flag = sign;
    /*if (rhs % BASE == 0) {
        number.insert_to_begin(rhs / BASE, negative_flag ? MAX_INT : 0);
        return *this;
    }*/
    if (negative_flag)
        *this = -(*this);
    big_integer result;
    size_t start = rhs / BASE;
    result.number.resize(start + number.size(), negative_flag ? MAX_INT : 0);
    auto res_data = result.number.get_data_and_check();
    auto data = number.get_data();
    rhs %= BASE;
    ui cf = 0;
    if (rhs == 0) {
        for (size_t i = 0; i < number.size(); i++)
            res_data[start + i] = data[i];
    } else {
        for (size_t i = 0; i < number.size(); i++) {
            res_data[start + i] = (data[i] << rhs) + cf;
            cf = cast_to_ui(data[i] >> (BASE - rhs));
        }
    }
    if (cf != 0)
        result.number.push_back(cf);
    if (negative_flag)
        result = -result;
    result.shrink();
    return *this = result;
}

big_integer operator<<(big_integer a, int b) {
    return a <<= b;
}

big_integer &big_integer::operator>>=(int rhs) {
    if (rhs < 0)
        throw "error! right value is negative";
    /*if (rhs % BASE == 0) {
        number.erase(number.begin(), number.begin() + (rhs / BASE));
        return *this;
    }*/
    bool negative_flag = sign;
    if (negative_flag)
        *this = -(*this);
    big_integer result;
    int gap = rhs / BASE;
    rhs %= BASE;
    result.number.resize(number.size() - gap + 1);
    auto res_data = result.number.get_data_and_check();
    auto data = number.get_data();
    if (rhs == 0) {
        for (int i = static_cast<int>(number.size()) - 1; i >= gap; i--)
            res_data[i - gap] = data[i];
    } else {
        ui cf = 0;
        for (int i = static_cast<int>(number.size()) - 1; i >= gap; i--) {
            res_data[i - gap] = (data[i] >> rhs) + cf;
            cf = cast_to_ui(data[i] << (BASE - rhs));
        }
    }
    result.shrink();
    //std::reverse(result.number.begin(), result.number.end());
    result.sign = sign;
    if (negative_flag)
        result = -result - 1;
    return *this = result;
}

big_integer operator>>(big_integer a, int b) {
    return a >>= b;
}

big_integer::ui big_integer::operator[](size_t pos) const {
    if (pos < number.size())
        return number[pos];
    return sign ? MAX_INT : 0;
}

void big_integer::shrink() {
    while (length() > 1 && ((sign && number.back() == MAX_INT) || (!sign && number.back() == 0)))
        number.pop_back();
}

std::string to_string(big_integer const &source) {
    if (source == 0)
        return "0";
    std::string result;
    result.resize(9 * source.length() + 2, '0');
    size_t str_it = 0;
    auto src_copy = source.sign ? -source : source;
    src_copy.shrink();
    src_copy.number.get_data_and_check();
    while (src_copy > 0) {
        auto slice = (src_copy % BASE_10)[0];
        for (size_t i = 0; i < 9; i++) {
            result[str_it++] = '0' + char(slice % 10);
            slice /= 10;
        }
        src_copy /= BASE_10;
    }
    while (str_it > 0 && result[str_it - 1] == '0')
        --str_it;
    if (source.sign)
        result[str_it++] = '-';
    result.resize(str_it);
    std::reverse(result.begin(), result.end());
    return result;
}
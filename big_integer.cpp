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
    number.clear();
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
    res.number.resize(number.size() + 1);
    for (size_t i = 0; i < number.size(); ++i)
        res.number[i] = ~number[i];
    res.shrink();
    res.add_one();
    res.sign = !sign;
    return res;
}

void big_integer::add_one() {
    bool cf = true;
    for (size_t i = 0; cf && i < number.size(); ++i) {
        number[i] += 1;
        cf = number[i] == 0;
    }
    if (cf)
        number.push_back(1);
}

big_integer &big_integer::operator+=(big_integer const &rhs) {
    ull cf = 0;
    big_integer result = rhs;
    size_t n = std::max(number.size(), result.number.size()) + 2;
    number.resize(n, sign ? MAX_INT : 0);
    result.number.resize(n, result.sign ? MAX_INT : 0);
    for (size_t it = 0; it < n; ++it) {
        cf = cf + number[it] + result.number[it];
        result.number[it] = cast_to_ui(cf);
        cf >>= BASE;
    }

    result.sign = sign;
    shrink();
    if (sign != rhs.sign) {
        int res = (sign ? abs_cmp(-(*this), rhs) : abs_cmp(*this, -rhs));
        if (res <= 0)
            result.sign = (res == 0 ? false : !result.sign);
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
    ull mul = 0, cf = 0;
    for (size_t i = offset; i < n; i++) {
        mul = static_cast<ull>((*this)[i - offset]) * x + cf;
        result.number[i] = cast_to_ui(mul);
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

    // normalize b
    ui k = static_cast<ui>(ceil(log2((static_cast<double>(cast_to_ui(SHIFT_BASE >> 1)) / b.number.back()))));
    b <<= k;
    (*this) <<= k;
    size_t m = length() - b.length();
    size_t n = b.length();
    big_integer q;
    q.number.resize(m + 1);
    b <<= (BASE * m);
    if (*this > b) {
        q.number[m] = 1;
        *this -= b;
    }

    for (int j = static_cast<int>(m - 1); j >= 0; j--) {
        q.number[j] = cast_to_ui(((static_cast<ull>((*this)[n + j]) << BASE) + (*this)[n + j - 1]) / b.number.back());
        b >>= BASE;
        *this -= b.mul_long_short(q[j], 0);

        // is executed twice in the worst case
        while (sign) {
            q.number[j]--;
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
    for (size_t i = 0; i < rhs_copy.length(); i++)
        *this += storage.mul_long_short(rhs_copy[i], i);
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
        if (a[i] != b[i])
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
    for (int i = static_cast<int>(a.length()) - 1; i >= 0; i--) {
        if (a[i] != b[i])
            return a.sign ^ (a[i] < b[i]);
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

int big_integer::abs_cmp(big_integer const &a, big_integer const &b) {
    assert(a.sign == b.sign);
    if (a.length() != b.length())
        return a.length() < b.length() ? -1 : 1;
    for (int i = static_cast<int>(a.length()) - 1; i >= 0; i--) {
        if (a[i] != b[i])
            return a[i] < b[i] ? -1 : 1;
    }
    return 0;
}

big_integer big_integer::operator~() const {
    return -(*this) - 1;
}

template<class FunctorT>
big_integer &big_integer::apply_bitwise_operation(big_integer const &rhs, FunctorT functor) {
    for (size_t i = 0; i < length(); i++)
        number[i] = functor(number[i], rhs[i]);
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
    if (rhs % BASE == 0) {
        std::vector<int> zeros(rhs / BASE, negative_flag ? MAX_INT : 0);
        number.insert(number.begin(), zeros.begin(), zeros.end());
        return *this;
    }
    if (negative_flag)
        *this = -(*this);
    big_integer result;
    result.number.resize(rhs / BASE);
    result.number.reserve(rhs / BASE + number.size() + 2);
    rhs %= BASE;
    ui cf = 0;
    for (auto &x: number) {
        result.number.push_back((x << rhs) + cf);
        cf = cast_to_ui(x >> (BASE - rhs));
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
    if (rhs % BASE == 0) {
        number.erase(number.begin(), number.begin() + (rhs / BASE));
        return *this;
    }
    bool negative_flag = sign;
    if (negative_flag)
        *this = -(*this);
    big_integer result;
    result.number.clear();
    result.number.reserve(length() + 2);
    int gap = rhs / BASE;
    rhs %= BASE;
    ui cf = 0;
    for (size_t i = length() - 1; static_cast<int>(i) >= gap; i--) {
        result.number.push_back(((*this)[i] >> rhs) + cf);
        cf = cast_to_ui((*this)[i] << (BASE - rhs));
    }
    std::reverse(result.number.begin(), result.number.end());
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
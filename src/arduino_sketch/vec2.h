#pragma once

// Can't use <type_traits> on arduino IDE so gotta do this...
template <typename T, typename U>
struct common_type {
    using type = decltype(T() + U());
};

template <typename T, typename U>
using common_type_t = typename common_type<T, U>::type;

template <typename T>
struct vec2 {
    T x;
    T y;

    vec2(T x = T(), T y = T()) : x(x), y(y) {}

    template <typename U>
    vec2(const vec2<U>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

    template <typename U>
    vec2<common_type_t<T, U>> operator+(const vec2<U>& other) const {
        using ResultType = common_type_t<T, U>;
        return vec2<ResultType>(
            static_cast<ResultType>(x) + static_cast<ResultType>(other.x),
            static_cast<ResultType>(y) + static_cast<ResultType>(other.y)
        );
    }

    template <typename U>
    vec2<common_type_t<T, U>> operator+(U other) const {
        using ResultType = common_type_t<T, U>;
        return vec2<ResultType>(
            static_cast<ResultType>(x) + static_cast<ResultType>(other),
            static_cast<ResultType>(y) + static_cast<ResultType>(other)
        );
    }

    template <typename U>
    friend vec2<common_type_t<T, U>> operator-(const vec2<T>& a, const vec2<U>& b) {
        using ResultType = common_type_t<T, U>;
        return vec2<ResultType>(
            static_cast<ResultType>(a.x) - static_cast<ResultType>(b.x),
            static_cast<ResultType>(a.y) - static_cast<ResultType>(b.y)
        );
    }

    template <typename U>
    friend vec2<common_type_t<T, U>> operator*(const vec2<T>& a, const vec2<U>& b) {
        using ResultType = common_type_t<T, U>;
        return vec2<ResultType>(
            static_cast<ResultType>(a.x) * static_cast<ResultType>(b.x),
            static_cast<ResultType>(a.y) * static_cast<ResultType>(b.y)
        );
    }

    template <typename U>
    friend vec2<common_type_t<T, U>> operator*(const vec2<T>& a, U b) {
        using ResultType = common_type_t<T, U>;
        return vec2<ResultType>(
            static_cast<ResultType>(a.x) * static_cast<ResultType>(b),
            static_cast<ResultType>(a.y) * static_cast<ResultType>(b)
        );
    }

    template <typename U>
    friend vec2<common_type_t<T, U>> operator/(const vec2<T>& a, const vec2<U>& b) {
        using ResultType = common_type_t<T, U>;
        return vec2<ResultType>(
            static_cast<ResultType>(a.x) / static_cast<ResultType>(b.x),
            static_cast<ResultType>(a.y) / static_cast<ResultType>(b.y)
        );
    }

    template <typename U>
    vec2<T>& operator+=(const vec2<U>& other) {
        x += static_cast<T>(other.x);
        y += static_cast<T>(other.y);
        return *this;
    }

    template <typename U>
    vec2<T>& operator+=(U other) {
        x += static_cast<T>(other);
        y += static_cast<T>(other);
        return *this;
    }


    template <typename U>
    vec2<T>& operator-=(const vec2<U>& other) {
        x -= static_cast<T>(other.x);
        y -= static_cast<T>(other.y);
        return *this;
    }

    template <typename U>
    vec2<T>& operator*=(const vec2<U>& other) {
        x *= static_cast<T>(other.x);
        y *= static_cast<T>(other.y);
        return *this;
    }

    template <typename U>
    vec2<T>& operator*=(U other) {
        x *= static_cast<T>(other.x);
        y *= static_cast<T>(other.y);
        return *this;
    }

    template <typename U>
    vec2<T>& operator/=(const vec2<U>& other) {
        x /= static_cast<T>(other.x);
        y /= static_cast<T>(other.y);
        return *this;
    }

};
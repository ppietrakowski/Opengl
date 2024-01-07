#pragma once

#include <chrono>

// Wrapper around chrono to easier access for time duration
class Duration {
    friend Duration operator+(const Duration& a, const Duration& b);
    friend Duration operator-(const Duration& a, const Duration& b);
    friend Duration operator/(const Duration& a, const Duration& b);
    friend Duration operator*(const Duration& a, const Duration& b);
    friend Duration operator/(const Duration& a, std::uint64_t scalar);

public:
    using duration_t = std::chrono::duration<std::uint64_t, std::nano>;
    using seconds_duration_t = std::chrono::duration<float>;
    using milliseconds_duration_t = std::chrono::duration<float, std::milli>;

    Duration() :
        DurationTime{duration_t::zero()} {
    }

    Duration(duration_t duration) :
        DurationTime{duration} {
    }

    Duration(std::chrono::nanoseconds duration) :
        DurationTime{std::chrono::duration_cast<duration_t>(duration)} {
    }

    Duration(const Duration&) = default;
    Duration& operator=(const Duration&) = default;

    float GetAsSeconds() const {
        return std::chrono::duration_cast<seconds_duration_t>(DurationTime).count();
    }

    float GetAsMilliseconds() const {
        return std::chrono::duration_cast<milliseconds_duration_t>(DurationTime).count();
    }

    std::uint64_t GetNanoSeconds() const {
        return DurationTime.count();
    }

    bool IsNonZero() const {
        return DurationTime != duration_t::zero();
    }

    bool IsZero() const {
        return DurationTime == duration_t::zero();
    }

private:
    duration_t DurationTime;
};

inline Duration operator+(const Duration& a, const Duration& b) {
    return a.DurationTime + b.DurationTime;
}

inline Duration operator-(const Duration& a, const Duration& b) {
    return a.DurationTime - b.DurationTime;
}

inline Duration operator*(const Duration& a, const Duration& b) {
    return a.DurationTime * b.DurationTime;
}

inline Duration operator/(const Duration& a, const Duration& b) {
    return Duration::duration_t{a.DurationTime / b.DurationTime};
}

inline Duration& operator+=(Duration& a, const Duration& b) {
    a = (a + b);
    return a;
}

inline Duration& operator-=(Duration& a, const Duration& b) {
    a = (a - b);
    return a;
}

inline Duration operator/(const Duration& a, std::uint64_t scalar) {
    return Duration::duration_t{a.DurationTime / scalar};
}
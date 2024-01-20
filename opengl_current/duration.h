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
        duration_time_{duration_t::zero()} {
    }

    Duration(duration_t duration) :
        duration_time_{duration} {
    }

    Duration(std::chrono::nanoseconds duration) :
        duration_time_{std::chrono::duration_cast<duration_t>(duration)} {
    }

    Duration(const Duration&) = default;
    Duration& operator=(const Duration&) = default;

    float GetSeconds() const {
        return std::chrono::duration_cast<seconds_duration_t>(duration_time_).count();
    }

    float GetMilliseconds() const {
        return std::chrono::duration_cast<milliseconds_duration_t>(duration_time_).count();
    }

    std::uint64_t GetNanoSeconds() const {
        return duration_time_.count();
    }

    bool IsNonZero() const {
        return duration_time_ != duration_t::zero();
    }

    bool IsZero() const {
        return duration_time_ == duration_t::zero();
    }

private:
    duration_t duration_time_;
};

inline Duration operator+(const Duration& a, const Duration& b) {
    return a.duration_time_ + b.duration_time_;
}

inline Duration operator-(const Duration& a, const Duration& b) {
    return a.duration_time_ - b.duration_time_;
}

inline Duration operator*(const Duration& a, const Duration& b) {
    return a.duration_time_ * b.duration_time_;
}

inline Duration operator/(const Duration& a, const Duration& b) {
    return Duration::duration_t{a.duration_time_ / b.duration_time_};
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
    return Duration::duration_t{a.duration_time_ / scalar};
}
#pragma once

#include <chrono>

using milliseconds_float_t = std::chrono::duration<float, std::milli>;

// Wrapper around chrono to easier access for time duration
class Duration
{
    friend Duration operator+(const Duration& a, const Duration& b);
    friend Duration operator-(const Duration& a, const Duration& b);
    friend Duration operator/(const Duration& a, const Duration& b);
    friend Duration operator*(const Duration& a, const Duration& b);
    friend Duration operator/(const Duration& a, std::uint64_t scalar);

public:
    using seconds_duration_t = std::chrono::duration<float>;
    using milliseconds_duration_t = std::chrono::duration<float, std::milli>;
    using duration_t = std::chrono::nanoseconds;

    Duration() :
        m_DurationTime{duration_t::zero()}
    {
    }

    template <typename Rep, typename Period>
    Duration(const std::chrono::duration<Rep, Period>& duration) :
        m_DurationTime{std::chrono::duration_cast<duration_t>(duration)}
    {
    }

    Duration(const Duration&) = default;
    Duration& operator=(const Duration&) = default;

    float GetSeconds() const
    {
        return std::chrono::duration_cast<seconds_duration_t>(m_DurationTime).count();
    }

    float GetMilliseconds() const
    {
        return std::chrono::duration_cast<milliseconds_duration_t>(m_DurationTime).count();
    }

    int64_t GetNanoSeconds() const
    {
        return m_DurationTime.count();
    }

    bool IsNonZero() const
    {
        return m_DurationTime != duration_t::zero();
    }

    bool IsZero() const
    {
        return m_DurationTime == duration_t::zero();
    }

    duration_t GetChronoNanoSeconds() const
    {
        return m_DurationTime;
    }

private:
    duration_t m_DurationTime;
};

inline Duration operator+(const Duration& a, const Duration& b)
{
    return a.m_DurationTime + b.m_DurationTime;
}

inline Duration operator-(const Duration& a, const Duration& b)
{
    return a.m_DurationTime - b.m_DurationTime;
}

inline Duration operator*(const Duration& a, const Duration& b)
{
    return a.m_DurationTime * b.m_DurationTime;
}

inline Duration operator/(const Duration& a, const Duration& b)
{
    return Duration::duration_t{a.m_DurationTime / b.m_DurationTime};
}

inline Duration& operator+=(Duration& a, const Duration& b)
{
    a = (a + b);
    return a;
}

inline Duration& operator-=(Duration& a, const Duration& b)
{
    a = (a - b);
    return a;
}

inline Duration operator/(const Duration& a, std::uint64_t scalar)
{
    return Duration::duration_t{a.m_DurationTime / scalar};
}
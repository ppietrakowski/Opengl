#pragma once


template<typename chrono_clock, typename duration = chrono_clock::duration>
class delta_clock_base
{
public:
    using time_point_t = chrono_clock::time_point;
    using duration_t = duration;

    delta_clock_base()
    {
        PerformTick();
        PerformTick();
    }

    void PerformTick()
    {
        m_Then = m_Now;
        m_Now = chrono_clock::now();
        m_Delta = std::chrono::duration_cast<duration>(m_Now - m_Then);
    }

    time_point_t GetNow() const
    {
        return m_Now;
    }

    duration_t GetDelta() const
    {
        return m_Delta;
    }

private:
    time_point_t m_Now;
    time_point_t m_Then;
    duration_t m_Delta;
};
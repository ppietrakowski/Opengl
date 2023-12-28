#pragma once

#include <cstdint>
#include <algorithm>

template <typename T>
class Buffer
{
public:
    Buffer(uint32_t initialCapacity) :
        m_Capacity{initialCapacity}
    {
        m_Data = new T[initialCapacity];
        m_Current = m_Data;
    }

    ~Buffer()
    {
        delete[] m_Data;
    }

    void Resize(uint32_t newCapacity)
    {
        if (newCapacity <= m_Capacity)
        {
            return;
        }

        uint32_t index = static_cast<uint32_t>(std::distance(m_Data, m_Current));

        T* data = new T[newCapacity];
        std::move(m_Data, m_Data + m_Capacity, data);

        delete[] m_Data;
        m_Data = data;
        m_Current = data[index];
        m_Capacity = newCapacity;
    }

    void AddInstance(const T& element)
    {
        if (GetSize() >= m_Capacity)
        {
            return;
        }

        *m_Current++ = element;
    }

    void ResetPtrToStart()
    {
        m_Current = m_Data;
    }

    T* GetRawData()
    {
        return m_Data;
    }

    const T* GetRawData() const
    {
        return m_Data;
    }

    uint32_t GetSize() const
    {
        return static_cast<uint32_t>(std::distance(m_Data, m_Current));
    }

    uint32_t GetSizeBytes() const
    {
        return GetSize() * sizeof(T);
    }

    uint32_t GetCapacity() const
    {
        return m_Capacity;
    }

    uint32_t GetCapacityBytes() const
    {
        return m_Capacity * sizeof(T);
    }

private:
    T* m_Data;
    T* m_Current;
    uint32_t m_Capacity;
};
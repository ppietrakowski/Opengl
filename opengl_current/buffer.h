#pragma once

#include <cstdint>
#include <algorithm>

template <typename T>
class Buffer
{
public:
    Buffer(int32_t initialCapacity) :
        Capacity{initialCapacity}
    {
        Data = new T[initialCapacity];
        Current = Data;
    }

    ~Buffer()
    {
        delete[] Data;
    }

    void Resize(int32_t newCapacity)
    {
        if (newCapacity <= Capacity)
        {
            return;
        }

        int32_t index = static_cast<int32_t>(std::distance(Data, Current));

        T* data = new T[newCapacity];
        std::move(Data, Data + Capacity, data);

        delete[] Data;
        Data = data;
        Current = data[index];
        Capacity = newCapacity;
    }

    void AddInstance(const T& element)
    {
        if (GetSize() >= Capacity)
        {
            return;
        }

        *Current++ = element;
    }

    void ResetPtrToStart()
    {
        Current = Data;
    }

    T* GetRawData()
    {
        return Data;
    }

    const T* GetRawData() const
    {
        return Data;
    }

    int32_t GetSize() const
    {
        return static_cast<int32_t>(std::distance(Data, Current));
    }

    int32_t GetSizeBytes() const
    {
        return GetSize() * sizeof(T);
    }

    int32_t GetCapacity() const
    {
        return Capacity;
    }

    int32_t GetCapacityBytes() const
    {
        return Capacity * sizeof(T);
    }

private:
    T* Data;
    T* Current;
    int32_t Capacity;
};
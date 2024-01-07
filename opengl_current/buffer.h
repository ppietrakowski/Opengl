#pragma once

#include <cstdint>
#include <algorithm>

template <typename T>
class Buffer
{
public:
    Buffer(std::uint32_t initial_capacity) :
        capacity_{initial_capacity}
    {
        data_ = new T[initial_capacity];
        current_ = data_;
    }

    ~Buffer()
    {
        delete[] data_;
    }

    void Resize(std::uint32_t new_capacity)
    {
        if (new_capacity <= capacity_)
        {
            return;
        }

        std::uint32_t index = static_cast<std::uint32_t>(std::distance(data_, current_));

        T* data = new T[new_capacity];
        std::move(data_, data_ + capacity_, data);

        delete[] data_;
        data_ = data;
        current_ = data[index];
        capacity_ = new_capacity;
    }

    void AddInstance(const T& element)
    {
        if (GetSize() >= capacity_)
        {
            return;
        }

        *current_++ = element;
    }

    void ResetPtrToStart()
    {
        current_ = data_;
    }

    T* GetRawData()
    {
        return data_;
    }

    const T* GetRawData() const
    {
        return data_;
    }

    std::uint32_t GetSize() const
    {
        return static_cast<std::uint32_t>(std::distance(data_, current_));
    }

    std::uint32_t GetSizeBytes() const
    {
        return GetSize() * sizeof(T);
    }

    std::uint32_t GetCapacity() const
    {
        return capacity_;
    }

    std::uint32_t GetCapacityBytes() const
    {
        return capacity_ * sizeof(T);
    }

private:
    T* data_;
    T* current_;
    std::uint32_t capacity_;
};
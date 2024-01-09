#pragma once

#include <cstdint>
#include <algorithm>

template <typename T>
class Buffer {
public:
    Buffer(std::int32_t initial_capacity) :
        capacity_{initial_capacity} {
        data_ = new T[initial_capacity];
        current_ = data_;
    }

    ~Buffer() {
        delete[] data_;
    }

    void Resize(std::int32_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        }

        std::int32_t index = static_cast<std::int32_t>(std::distance(data_, current_));

        T* data = new T[new_capacity];
        std::move(data_, current_, data);

        delete[] data_;
        data_ = data;
        current_ = &data[index];
        capacity_ = new_capacity;
    }

    void AddInstance(const T& element) {
        if (GetSize() >= capacity_) {
            return;
        }

        *current_++ = element;
    }

    void ResetPtrToStart() {
        current_ = data_;
    }

    T* GetRawData() {
        return data_;
    }

    const T* GetRawData() const {
        return data_;
    }

    std::int32_t GetSize() const {
        return static_cast<std::int32_t>(std::distance(data_, current_));
    }

    std::int32_t GetSizeBytes() const {
        return GetSize() * sizeof(T);
    }

    std::int32_t GetCapacity() const {
        return capacity_;
    }

    std::int32_t GetCapacityBytes() const {
        return capacity_ * sizeof(T);
    }

private:
    T* data_;
    T* current_;
    std::int32_t capacity_;
};
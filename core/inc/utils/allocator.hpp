#pragma once
#ifndef WFCPP__UTILS__ALLOCATOR_HPP_
#define WFCPP__UTILS__ALLOCATOR_HPP_


#include <cstddef>
#include <limits>
#include <new>


namespace wfcpp
{
inline namespace utils
{
class AllocatorMetrics
{
public:
    inline void alloc(std::size_t bytes) noexcept
    {
        m_bytes_allocated += bytes;
    }
    inline void dealloc(std::size_t bytes) noexcept
    {
        m_bytes_deallocated += bytes;
    }

    inline std::size_t bytes() const noexcept
    {
        return allocated() - deallocated();
    }
    inline std::size_t allocated() const noexcept
    {
        return m_bytes_allocated;
    }
    inline std::size_t deallocated() const noexcept
    {
        return m_bytes_deallocated;
    }

private:
    std::size_t m_bytes_allocated = 0;
    std::size_t m_bytes_deallocated = 0;
};

template<typename T>
class CustomAllocator
{
public:
    using value_type = T;

    inline value_type* allocate(std::size_t size)
    {
        if (size > std::numeric_limits<std::size_t>::max() / sizeof(value_type))
        {
            throw std::bad_alloc();
        }

        value_type* buffer = new value_type[size];
        if (buffer == nullptr)
        {
            throw std::bad_alloc();
        }

        m_metrics.alloc(size * sizeof(value_type));

        return buffer;
    }

    inline void deallocate(value_type* ptr, std::size_t size) noexcept
    {
        delete[] ptr;

        m_metrics.dealloc(size * sizeof(value_type));
    }

    template<typename U>
    inline bool operator==(const CustomAllocator<U>&)
    {
        return true;
    }
    template<typename U>
    inline bool operator!=(const CustomAllocator<U>&)
    {
        return false;
    }

    static inline std::size_t bytes() noexcept
    {
        return m_metrics.bytes();
    }
    static inline std::size_t allocated() noexcept
    {
        return m_metrics.allocated();
    }
    static inline std::size_t deallocated() noexcept
    {
        return m_metrics.deallocated();
    }

private:
    // `static inline` needs C++17
    static inline AllocatorMetrics m_metrics = {};
};
}  // inline namespace utils
}  // namespace wfcpp


#endif  // WFCPP__UTILS__ALLOCATOR_HPP_

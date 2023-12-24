#pragma once
#ifndef WFCPP__UTILS__ARRAY_HPP_
#define WFCPP__UTILS__ARRAY_HPP_


#include <cassert>
#include <array>

#include "allocator.hpp"


namespace wfcpp
{
inline namespace utils
{
template<typename T>
class Array2D
{
public:
    using value_type = T;
    using buffer_type = std::vector<value_type, CustomAllocator<value_type>>;

    inline Array2D(std::size_t height, std::size_t width) noexcept
        : m_height(height), m_width(width), m_buffer(width * height) {}
    inline Array2D(std::size_t height, std::size_t width, value_type value)
        noexcept
        : m_height(height), m_width(width), m_buffer(width * height, value) {}

    inline value_type& operator()(std::size_t i, std::size_t j) noexcept
    {
        return m_buffer.at(idx(i, j));
    }
    inline const value_type& operator()(std::size_t i, std::size_t j) const
        noexcept
    {
        return m_buffer.at(idx(i, j));
    }

    inline bool operator==(const Array2D<value_type>& rhs) const noexcept
    {
        if (m_height != rhs.m_height || m_width != rhs.m_width)
        {
            return false;
        }

        return m_buffer == rhs.m_buffer;
    }

    inline const std::size_t& height() const noexcept
    {
        return m_height;
    }
    inline const std::size_t& width() const noexcept
    {
        return m_width;
    }

    inline buffer_type& buffer() noexcept
    {
        return m_buffer;
    }
    inline const buffer_type& buffer() const noexcept
    {
        return m_buffer;
    }

    inline Array2D<value_type> reflected() const noexcept
    {
        Array2D<value_type> result = Array2D<value_type>(m_width, m_height);
        for (std::size_t y = 0; y < m_height; y++)
        {
            for (std::size_t x = 0; x < m_width; x++)
            {
                result(y, x) = (*this)(y, m_width - 1 - x);
            }
        }
        return result;
    }

    inline Array2D<value_type> rotated() const noexcept
    {
        Array2D<value_type> result = Array2D<value_type>(m_width, m_height);
        for (std::size_t y = 0; y < m_width; y++)
        {
            for (std::size_t x = 0; x < m_height; x++)
            {
                result(y, x) = (*this)(x, m_width - 1 - y);
            }
        }
        return result;
    }

    inline Array2D<value_type> get_sub_array(
        std::size_t y, std::size_t x,
        std::size_t sub_width, std::size_t sub_height) const noexcept
    {
        Array2D<value_type> sub_array_2d
            = Array2D<value_type>(sub_width, sub_height);
        for (std::size_t ki = 0; ki < sub_height; ki++)
        {
            for (std::size_t kj = 0; kj < sub_width; kj++)
            {
                sub_array_2d(ki, kj)
                    = (*this)((y + ki) % m_height, (x + kj) % m_width);
            }
        }
        return sub_array_2d;
    }

private:
    inline std::size_t idx(std::size_t i, std::size_t j) const noexcept
    {
        // TODO: check std::out_of_range
        return i * m_width + j;
    }

    std::size_t m_height = 0;
    std::size_t m_width = 0;
    buffer_type m_buffer;
};

template<typename T>
class Array3D
{
public:
    using value_type = T;
    using buffer_type = std::vector<value_type, CustomAllocator<value_type>>;

    inline Array3D(std::size_t height, std::size_t width, std::size_t depth)
        noexcept
        : m_height(height), m_width(width), m_depth(depth),
          m_buffer(width * height * depth) {}
    inline Array3D(std::size_t height, std::size_t width, std::size_t depth,
                   value_type value) noexcept
        : m_height(height), m_width(width), m_depth(depth),
          m_buffer(width * height * depth, value) {}

    inline value_type& operator()(std::size_t i, std::size_t j, std::size_t k)
        noexcept
    {
        return m_buffer.at(idx(i, j, k));
    }
    inline const value_type& operator()(std::size_t i, std::size_t j,
                                     std::size_t k) const noexcept
    {
        return m_buffer.at(idx(i, j, k));
    }

    inline bool operator==(const Array3D<value_type>& rhs) const noexcept
    {
        if (m_height != rhs.m_height || m_width != rhs.m_width ||
            m_depth != rhs.m_depth)
        {
            return false;
        }

        return m_buffer == rhs.m_buffer;
    }

private:
    inline std::size_t idx(std::size_t i, std::size_t j, std::size_t k) const
        noexcept
    {
        return i * m_width * m_depth + j * m_depth + k;
    }

    std::size_t m_height = 0;
    std::size_t m_width = 0;
    std::size_t m_depth = 0;
    buffer_type m_buffer;
};
}  // inline namespace utils
}  // namespace wfcpp


template<typename T>
class std::hash<wfcpp::Array2D<T>>
{
public:
    inline std::size_t operator()(const wfcpp::Array2D<T>& a) const noexcept
    {
        std::size_t seed = a.buffer().size();
        for (const T& i : a.buffer())
        {
            seed ^= std::hash<T>()(i) + static_cast<std::size_t>(0x9e3779b9)
                    + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
};


#endif  // WFCPP__UTILS__ARRAY_HPP_

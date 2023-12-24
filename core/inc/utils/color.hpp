#pragma once
#ifndef WFCPP__UTILS__COLOR_HPP_
#define WFCPP__UTILS__COLOR_HPP_


#include <functional>


namespace wfcpp
{
inline namespace utils
{
struct Color
{
    unsigned char r, g, b;

    Color(): Color(0, 0, 0) {};
    Color(unsigned char r, unsigned char g, unsigned char b)
        : r(r), g(g), b(b) {};

    inline bool operator==(const Color& c) const noexcept
    {
        return r == c.r && g == c.g && b == c.b;
    }
    inline bool operator!=(const Color &c) const noexcept
    {
        return !(c == *this);
    }
};
}  // inline namespace utils
}  // namespace wfcpp


template<>
class std::hash<wfcpp::Color>
{
public:
    inline std::size_t operator()(const wfcpp::Color& c) const noexcept
    {
        return   (static_cast<std::size_t>(c.r))
               + (static_cast<std::size_t>(c.g) << 8)
               + (static_cast<std::size_t>(c.b) << 16);
    }
};


#endif  // WFCPP__UTILS__COLOR_HPP_

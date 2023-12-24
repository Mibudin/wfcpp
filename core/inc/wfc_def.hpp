#pragma once
#ifndef WFCPP__WFC_DEF_HPP_
#define WFCPP__WFC_DEF_HPP_


namespace wfcpp
{
constexpr int directions_x[4] = { 0, -1,  1,  0};
constexpr int directions_y[4] = {-1,  0,  0,  1};

constexpr unsigned int get_opposite_direction(unsigned int direction) noexcept
{
  return 3 - direction;
}
}  // namespace wfcpp


#endif  // WFCPP__WFC_DEF_HPP_
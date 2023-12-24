#pragma once
#ifndef WFCPP__PROPAGATOR_HPP_
#define WFCPP__PROPAGATOR_HPP_


#include <vector>
#include <array>
#include <tuple>

#include "wave.hpp"


namespace wfcpp
{
using PropagatorState = std::vector<std::array<std::vector<unsigned int>, 4>>;

class Propagator
{
public:
    inline Propagator(std::size_t wave_height, std::size_t wave_width,
                      bool periodic_output, PropagatorState propagator_state)
        noexcept
        : m_patterns_size(propagator_state.size()),
          m_propagator_state(propagator_state),
          m_wave_width(wave_width),
          m_wave_height(wave_height),
          m_periodic_output(periodic_output),
          m_compatible(wave_height, wave_width, m_patterns_size)
    {
        init_compatible();
    }

    inline void add_to_propagator(std::size_t y, std::size_t x,
                                  unsigned pattern) noexcept
    {
        std::array<int, 4> temp = {};
        m_compatible(y, x, pattern) = temp;
        m_propagating.emplace_back(y, x, pattern);
    }

    void propagate(Wave& wave) noexcept;

private:
    void init_compatible() noexcept;

    const std::size_t m_patterns_size;
    PropagatorState m_propagator_state;
    const std::size_t m_wave_height;
    const std::size_t m_wave_width;
    const bool m_periodic_output;
    std::vector<std::tuple<std::size_t, std::size_t, unsigned int>>
        m_propagating;
    Array3D<std::array<int, 4>> m_compatible;
};
}  // namespace wfcpp


#endif  // WFCPP__PROPAGATOR_HPP_

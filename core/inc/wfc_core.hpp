#pragma once
#ifndef WFCPP__WFC_CORE_HPP_
#define WFCPP__WFC_CORE_HPP_


#include <random>
#include <optional>

#include "utils/array.hpp"
#include "wave.hpp"
#include "propagator.hpp"


namespace wfcpp
{
enum class ObserveStatus
{
    success, failure, to_continue
};

class WFCCore
{
public:
    WFCCore(bool periodic_output, int seed,
            std::vector<double> patterns_frequencies,
            PropagatorState propagator_state,
            std::size_t wave_height, std::size_t wave_width) noexcept;

    std::optional<Array2D<unsigned int>> run() noexcept;

    ObserveStatus observe() noexcept;

    inline void propagate() noexcept
    {
        m_propagator.propagate(m_wave);
    };

    inline void remove_wave_pattern(std::size_t i, std::size_t j,
                                    unsigned int pattern) noexcept
    {
        if (m_wave.get(i, j, pattern))
        {
            m_wave.set(i, j, pattern, false);
            m_propagator.add_to_propagator(i, j, pattern);
        }
    }

private:
    Array2D<unsigned int> wave_to_output() const noexcept;

    std::minstd_rand m_gen;
    const std::vector<double> m_patterns_frequencies;
    Wave m_wave;
    const std::size_t m_n_patterns;
    Propagator m_propagator;
};
}  // namespace wfcpp


#endif  // WFCPP__WFC_CORE_HPP_

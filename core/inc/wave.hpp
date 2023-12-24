#pragma once
#ifndef WFCPP__WAVE_HPP_
#define WFCPP__WAVE_HPP_


#include <random>

#include "utils/array.hpp"


namespace wfcpp
{
struct EntropyMemoisation
{
    // TODO: why not use Array2D
    // TODO: different order
    std::vector<double> plogp_sum;
    std::vector<double> p_sum;
    std::vector<double> sum_log;
    std::vector<unsigned int> n_patterns;
    std::vector<double> entropy;
};

class Wave
{
public:
    Wave(std::size_t height, std::size_t width,
         const std::vector<double>& patterns_frequencies) noexcept;

    inline bool get(std::size_t idx, unsigned pattern) const noexcept
    {
        return m_data(idx, pattern);
    }
    inline bool get(std::size_t i, std::size_t j, unsigned pattern) const
        noexcept
    {
        return get(i * m_width + j, pattern);
    }
    void set(std::size_t idx, unsigned pattern, bool value) noexcept;
    inline void set(std::size_t i, std::size_t j, unsigned pattern, bool value)
        noexcept
    {
        set(i * m_width + j, pattern, value);
    }

    int get_min_entropy(std::minstd_rand &gen) const noexcept;

    inline const std::size_t& width() const noexcept
    {
        return m_width;
    }
    inline const std::size_t& height() const noexcept
    {
        return m_height;
    }
    inline const std::size_t& size() const noexcept
    {
        return m_size;
    }

private:
    const std::size_t m_width;
    const std::size_t m_height;
    const std::size_t m_size;

    const std::vector<double> m_patterns_frequencies;
    const std::vector<double> m_patterns_frequencies_plogp;
    const double m_min_abs_half_plogp;
    EntropyMemoisation m_memoisation;

    bool m_is_impossible;
    const std::size_t m_n_patterns;
    Array2D<unsigned char> m_data;  // TODO: why not use Array3D
};
}  // namespace wfcpp


#endif  // WFCPP__WAVE_HPP_

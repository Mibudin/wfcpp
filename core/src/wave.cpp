#include "wave.hpp"

#include <limits>


namespace wfcpp
{
namespace
{
std::vector<double> get_plogp(const std::vector<double>& distribution) noexcept
{
    std::vector<double> plogp;
    for (const double& p : distribution)
    {
        plogp.emplace_back(p * std::log(p));
    }
    return plogp;
}

double get_min_abs_half(const std::vector<double>& vec) noexcept
{
    double min_abs_half = std::numeric_limits<double>::infinity();
    for (const double& val : vec)
    {
        min_abs_half = std::min(min_abs_half, std::abs(val / 2.0));
    }
    return min_abs_half;
}
}  // namespace

Wave::Wave(std::size_t height, std::size_t width,
           const std::vector<double>& patterns_frequencies) noexcept
    : m_patterns_frequencies(patterns_frequencies),
      m_patterns_frequencies_plogp(get_plogp(patterns_frequencies)),
      m_min_abs_half_plogp(get_min_abs_half(m_patterns_frequencies_plogp)),
      m_is_impossible(false),
      m_n_patterns(patterns_frequencies.size()),
      m_data(width * height, m_n_patterns, 1),
      m_width(width),
      m_height(height),
      m_size(width * height)
{
    double base_entropy = 0.0;
    double base_s = 0.0;
    for (unsigned int k = 0; k < m_n_patterns; k++)
    {
        base_entropy += m_patterns_frequencies_plogp.at(k);
        base_s += m_patterns_frequencies.at(k);
    }
    double log_base_s = std::log(base_s);
    double entropy_base = log_base_s - base_entropy / base_s;

    m_memoisation.plogp_sum = std::vector<double>(m_size, base_entropy);
    m_memoisation.p_sum = std::vector<double>(m_size, base_s);
    m_memoisation.sum_log = std::vector<double>(m_size, log_base_s);
    m_memoisation.n_patterns =
        std::vector<unsigned>(m_size, static_cast<unsigned>(m_n_patterns));
    m_memoisation.entropy = std::vector<double>(m_size, entropy_base);
}

void Wave::set(std::size_t idx, unsigned pattern, bool value) noexcept
{
    if (m_data(idx, pattern) == value) {
        return;
    }

    m_data(idx, pattern) = value;

    m_memoisation.plogp_sum.at(idx) -= m_patterns_frequencies_plogp.at(pattern);
    m_memoisation.p_sum.at(idx) -= m_patterns_frequencies.at(pattern);
    m_memoisation.sum_log.at(idx) = std::log(m_memoisation.p_sum.at(idx));
    m_memoisation.n_patterns.at(idx)--;
    m_memoisation.entropy.at(idx)
        = m_memoisation.sum_log.at(idx)
        - m_memoisation.plogp_sum.at(idx) / m_memoisation.p_sum.at(idx);

    if (m_memoisation.n_patterns.at(idx) == 0)
    {
        m_is_impossible = true;
    }
}

int Wave::get_min_entropy(std::minstd_rand &gen) const noexcept
{
    // TODO: enum those return value

    if (m_is_impossible) {
        return -2;
    }

    std::uniform_int_distribution<> dist(0, m_min_abs_half_plogp);

    double entropy_min = std::numeric_limits<double>::infinity();
    int arg_min = -1;

    for (std::size_t idx = 0; idx < m_size; idx++)
    {
        double n_patterns_local = m_memoisation.n_patterns.at(idx);
        if (n_patterns_local == 1)
        {
            continue;
        }

        double entropy = m_memoisation.entropy.at(idx);
        if (entropy <= entropy_min)
        {
            double noise = dist(gen);
            if (entropy + noise < entropy_min)
            {
                entropy_min = entropy + noise;
                arg_min = idx;
            }
        }
    }

    return arg_min;
}
}  // namespace wfcpp

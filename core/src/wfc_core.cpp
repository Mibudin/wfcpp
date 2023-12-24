#include "wfc_core.hpp"

#include <numeric>
namespace wfcpp
{
namespace
{
std::vector<double>& normalize(std::vector<double>& vec)
{
    const double sum_weights = std::accumulate(vec.cbegin(), vec.cend(), 0.0);
    const double inversed_sum_weights = 1.0 / sum_weights;

    for (double& weight : vec)
    {
        weight *= inversed_sum_weights;
    }

    return vec;
}
}  // namespace

WFCCore::WFCCore(bool periodic_output, int seed,
                 std::vector<double> patterns_frequencies,
                 PropagatorState propagator_state,
                 std::size_t wave_height, std::size_t wave_width) noexcept
    : m_gen(seed),
      m_patterns_frequencies(normalize(patterns_frequencies)),
      m_wave(wave_height, wave_width, patterns_frequencies),
      m_n_patterns(propagator_state.size()),
      m_propagator(wave_height, wave_width, periodic_output, propagator_state)
{}

std::optional<Array2D<unsigned int>> WFCCore::run() noexcept
{
    while (true)
    {
        ObserveStatus result = observe();

        using enum ObserveStatus;
        switch (result)
        {
        case success:
            return wave_to_output();

        case failure:
            return std::nullopt;

        case to_continue:
            m_propagator.propagate(m_wave);
            continue;
        }
    }
}

ObserveStatus WFCCore::observe() noexcept
{
    int arg_min = m_wave.get_min_entropy(m_gen);

    if (arg_min == -2)
    {
        return ObserveStatus::failure;
    }
    if (arg_min == -1)
    {
        // FIXME:
        // wave_to_output();
        return ObserveStatus::success;
    }

    double s = 0.0;
    for (unsigned int k = 0; k < m_n_patterns; k++)
    {
        s += m_wave.get(arg_min, k) ? m_patterns_frequencies.at(k) : 0;
    }

    std::uniform_real_distribution<> dist(0, s);
    double random_value = dist(m_gen);
    std::size_t chosen_value = m_n_patterns - 1;

    for (unsigned int k = 0; k < m_n_patterns; k++)
    {
        random_value -=
            m_wave.get(arg_min, k) ? m_patterns_frequencies.at(k) : 0;
        if (random_value <= 0)
        {
            chosen_value = k;
            break;
        }
    }

    for (unsigned int k = 0; k < m_n_patterns; k++)
    {
        if (m_wave.get(arg_min, k) != (k == chosen_value))
        {
            m_propagator.add_to_propagator(
                arg_min / m_wave.width(), arg_min % m_wave.width(), k);
            m_wave.set(arg_min, k, false);
        }
    }

    return ObserveStatus::to_continue;
}

Array2D<unsigned int> WFCCore::wave_to_output() const noexcept
{
    Array2D<unsigned int> output_patterns(m_wave.height(), m_wave.width());
    for (std::size_t idx = 0; idx < m_wave.size(); idx++)
    {
        for (unsigned int k = 0; k < m_n_patterns; k++)
        {
            if (m_wave.get(idx, k))
            {
                output_patterns.buffer().at(idx) = k;
            }
        }
    }
    return output_patterns;
}
}  // namespace wfcpp

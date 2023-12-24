#pragma once
#ifndef WFCPP__OVERLAPPING_MODEL_HPP_
#define WFCPP__OVERLAPPING_MODEL_HPP_


#include <unordered_map>

#include "wfc_def.hpp"
#include "wfc_core.hpp"


namespace wfcpp
{
struct OverlappingModelOptions
{
    unsigned int pattern_size;
    bool periodic_input;
    bool periodic_output;
    std::size_t out_height;
    std::size_t out_width;
    unsigned int symmetry;
    bool ground;

    inline std::size_t get_wave_height() const noexcept
    {
        return periodic_output ? out_height : out_height - pattern_size + 1;
    }

    inline std::size_t get_wave_width() const noexcept
    {
        return periodic_output ? out_width : out_width - pattern_size + 1;
    }
};

template<typename T>
class OverlappingModel
{
public:
    using pixel_type = T;

    inline OverlappingModel(
        const Array2D<pixel_type>& input,
        const OverlappingModelOptions& options,
        int seed) noexcept
        : OverlappingModel(input, options, seed, get_patterns(input, options))
    {}

    inline bool set_pattern(const Array2D<pixel_type>& pattern, std::size_t i,
                     std::size_t j) noexcept
    {
        auto pattern_id = get_pattern_id(pattern);

        if (pattern_id == std::nullopt ||
            i >= m_options.get_wave_height() ||
            j >= m_options.get_wave_width())
        {
            return false;
        }

        set_pattern(pattern_id, i, j);
        return true;
    }

    inline std::optional<Array2D<pixel_type>> run() noexcept
    {
        std::optional<Array2D<unsigned int>> result = m_wfc.run();
        if (result.has_value())
        {
            return to_image(*result);
        }
        return std::nullopt;
    }

private:
    inline OverlappingModel(
        const Array2D<pixel_type>& input,
        const OverlappingModelOptions& options,
        const int& seed,
        const std::pair<std::vector<Array2D<pixel_type>>,
                        std::vector<double>>& patterns,
        const PropagatorState& propagator_state) noexcept
        : m_input(input),
          m_options(options),
          m_patterns(patterns.first),
          m_wfc(
            options.periodic_output,
            seed,
            patterns.second,
            propagator_state,
            options.get_wave_height(),
            options.get_wave_width())
    {
        if (options.ground)
        {
            init_ground(m_wfc, input, patterns.first, options);
        }
    }

    inline OverlappingModel(
        const Array2D<pixel_type>& input,
        const OverlappingModelOptions& options,
        const int& seed,
        const std::pair<std::vector<Array2D<pixel_type>>,
                        std::vector<double>>& patterns) noexcept
        : OverlappingModel(input, options, seed, patterns,
                           generate_compatible(patterns.first)) {}

    inline void init_ground(WFCCore& wfc, const Array2D<pixel_type>& input,
                            const std::vector<Array2D<pixel_type>>& patterns,
                            const OverlappingModelOptions& options) noexcept
    {
        unsigned int ground_pattern_id
            = get_ground_pattern_id(input, patterns, options);

        for (std::size_t j = 0; j < options.get_wave_width(); j++)
        {
            set_pattern(ground_pattern_id, options.get_wave_height() - 1, j);
        }

        for (std::size_t i = 0; i < options.get_wave_height() - 1; i++)
        {
            for (std::size_t j = 0; j < options.get_wave_width(); j++)
            {
                wfc.remove_wave_pattern(i, j, ground_pattern_id);
            }
        }

        wfc.propagate();
    }

    static unsigned int get_ground_pattern_id(
        const Array2D<pixel_type>& input,
        const std::vector<Array2D<pixel_type>>& patterns,
        const OverlappingModelOptions& options) noexcept
    {
        Array2D<pixel_type> ground_pattern = input.get_sub_array(
            input.height() - 1, input.width() / 2,
            options.pattern_size, options.pattern_size);

        // TODO: use std::find?
        for (unsigned int i = 0; i < patterns.size(); i++)
        {
            if (ground_pattern == patterns.at(i))
            {
                return i;
            }
        }

        assert(false);
        return 0;
    }

    static inline
    std::pair<std::vector<Array2D<pixel_type>>, std::vector<double>>
    get_patterns(const Array2D<pixel_type>& input,
                 const OverlappingModelOptions& options) noexcept
    {
        std::unordered_map<Array2D<pixel_type>, unsigned int> patterns_id;
        std::vector<Array2D<pixel_type>> patterns;

        std::vector<double> patterns_weight;

        std::vector<Array2D<pixel_type>> symmetries(
            8, Array2D<pixel_type>(options.pattern_size, options.pattern_size)
        );
        std::size_t max_i = options.periodic_input
                            ? input.height()
                            : input.height() - options.pattern_size + 1;
        std::size_t max_j = options.periodic_input
                            ? input.width()
                            : input.width() - options.pattern_size + 1;

        for (std::size_t i = 0; i < max_i; i++)
        {
            for (std::size_t j = 0; j < max_j; j++)
            {
                symmetries[0].buffer() = input.get_sub_array(
                    i, j, options.pattern_size, options.pattern_size).buffer();
                symmetries[1].buffer() = symmetries[0].reflected().buffer();
                symmetries[2].buffer() = symmetries[0].rotated().buffer();
                symmetries[3].buffer() = symmetries[2].reflected().buffer();
                symmetries[4].buffer() = symmetries[2].rotated().buffer();
                symmetries[5].buffer() = symmetries[4].reflected().buffer();
                symmetries[6].buffer() = symmetries[4].rotated().buffer();
                symmetries[7].buffer() = symmetries[6].reflected().buffer();

                for (unsigned int k = 0; k < options.symmetry; k++)
                {
                    auto res = patterns_id.insert(
                        std::make_pair(symmetries.at(k), patterns.size()));

                    if (!res.second)
                    {
                        patterns_weight.at(res.first->second) += 1;
                    }
                    else
                    {
                        patterns.emplace_back(symmetries[k]);
                        patterns_weight.emplace_back(1);
                    }
                }
            }
        }

        return {patterns, patterns_weight};
    }

    static bool agrees(const Array2D<pixel_type>& pattern1,
                       const Array2D<pixel_type>& pattern2, int dy, int dx)
        noexcept
    {
        unsigned int x_min = dx < 0 ? 0 : dx;
        unsigned int x_max = dx < 0 ? dx + pattern2.width() : pattern1.width();
        unsigned int y_min = dy < 0 ? 0 : dy;
        unsigned int y_max = dy < 0 ? dy + pattern2.height()
                                    : pattern1.width();

        for (unsigned int y = y_min; y < y_max; y++)
        {
            for (unsigned int x = x_min; x < x_max; x++)
            {
                if (pattern1(y, x) != pattern2(y - dy, x - dx))
                {
                    return false;
                }
            }
        }
        return true;
    }

    static inline std::vector<std::array<std::vector<unsigned>, 4>>
    generate_compatible(const std::vector<Array2D<T>> &patterns) noexcept
    {
        std::vector<std::array<std::vector<unsigned int>, 4>> compatible
            = std::vector<std::array<std::vector<unsigned int>, 4>>
                (patterns.size());

        // Iterate on every dy, dx, pattern1 and pattern2
        for (unsigned int pattern1 = 0; pattern1 < patterns.size(); pattern1++)
        {
            for (unsigned int direction = 0; direction < 4; direction++)
            {
                for (unsigned int pattern2 = 0; pattern2 < patterns.size();
                     pattern2++)
                {
                    if (agrees(patterns[pattern1], patterns[pattern2],
                            directions_y[direction], directions_x[direction]))
                    {
                        compatible.at(pattern1).at(direction)
                            .emplace_back(pattern2);
                    }
                }
            }
        }

        return compatible;
    }

    inline Array2D<pixel_type> to_image(
        const Array2D<unsigned int>& output_patterns) const noexcept
    {
        Array2D<pixel_type> output
            = Array2D<pixel_type>(m_options.out_height, m_options.out_width);

        if (m_options.periodic_output)
        {
            for (std::size_t y = 0; y < m_options.get_wave_height(); y++)
            {
                for (std::size_t x = 0; x < m_options.get_wave_width(); x++)
                {
                    output(y, x) = m_patterns.at(output_patterns(y, x))(0, 0);
                }
            }
        }
        else
        {
            for (std::size_t y = 0; y < m_options.get_wave_height(); y++)
            {
                for (std::size_t x = 0; x < m_options.get_wave_width(); x++)
                {
                    output(y, x) = m_patterns.at(output_patterns(y, x))(0, 0);
                }
            }

            for (std::size_t y = 0; y < m_options.get_wave_height(); y++)
            {
                const Array2D<pixel_type>& pattern
                    = m_patterns.at(
                        output_patterns(y, m_options.get_wave_width() - 1));
                for (std::size_t dx = 1; dx < m_options.pattern_size; dx++)
                {
                    output(y, m_options.get_wave_width() - 1 + dx)
                        = pattern(0, dx);
                }
            }

            for (std::size_t x = 0; x < m_options.get_wave_width(); x++)
            {
                const Array2D<pixel_type>& pattern
                    = m_patterns.at(
                        output_patterns(m_options.get_wave_height() - 1, x));
                for (std::size_t dy = 1; dy < m_options.pattern_size; dy++)
                {
                    output(m_options.get_wave_height() - 1 + dy, x)
                        = pattern(dy, 0);
                }
            }

            const Array2D<pixel_type>& pattern = m_patterns.at(
                output_patterns(m_options.get_wave_height() - 1,
                                m_options.get_wave_width() - 1));
            for (std::size_t dy = 1; dy < m_options.pattern_size; dy++)
            {
                for (std::size_t dx = 1; dx < m_options.pattern_size; dx++)
                {
                    output(
                        m_options.get_wave_height() - 1 + dy,
                        m_options.get_wave_width() - 1 + dx)
                        = pattern(dy, dx);
                }
            }
        }

        return output;
    }

    std::optional<unsigned int> get_pattern_id(
        const Array2D<pixel_type>& pattern) const noexcept
    {
        // TODO: handling pointers
        unsigned int* pattern_id
            = std::find(m_patterns.cbegin(), m_patterns.cend(), pattern);

        if (pattern_id != m_patterns.cend())
        {
            return *pattern_id;
        }
        return std::nullopt;
    }

    void set_pattern(unsigned int pattern_id, unsigned int i, unsigned int j)
        noexcept
    {
        for (unsigned int p = 0; p < m_patterns.size(); p++)
        {
            if (pattern_id != p)
            {
                m_wfc.remove_wave_pattern(i, j, p);
            }
        }
    }

    Array2D<pixel_type> m_input;
    OverlappingModelOptions m_options;
    std::vector<Array2D<pixel_type>> m_patterns;
    WFCCore m_wfc;
};
}  // namespace wfcpp


#endif  // WFCPP__OVERLAPPING_MODEL_HPP_

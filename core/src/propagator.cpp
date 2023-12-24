#include "propagator.hpp"

#include "wfc_def.hpp"


namespace wfcpp
{
void Propagator::propagate(Wave& wave) noexcept
{
    // TODO: types of coordinates
    while (m_propagating.size() != 0)
    {
        std::size_t x1, y1;
        unsigned int pattern;
        std::tie(y1, x1, pattern) = m_propagating.back();
        m_propagating.pop_back();

        for (unsigned int direction = 0; direction < 4; direction++)
        {
            int dx = directions_x[direction];
            int dy = directions_y[direction];
            int y2, x2;

            if (m_periodic_output)
            {
                x2 = (static_cast<int>(x1) + dx
                      + static_cast<int>(wave.width())) % wave.width();
                y2 = (static_cast<int>(y1) + dy
                      + static_cast<int>(wave.height())) % wave.height();
            }
            else
            {
                x2 = x1 + dx;
                y2 = y1 + dy;
                if (x2 < 0 || x2 >= static_cast<int>(wave.width()) ||
                    y2 < 0 || y2 >= static_cast<int>(wave.height()))
                {
                    continue;
                }
            }

            unsigned int idx2 = x2 + y2 * wave.width();
            const std::vector<unsigned int>& patterns
                = m_propagator_state.at(pattern).at(direction);

            for (const unsigned int& p : patterns)
            {
                std::array<int, 4>& value = m_compatible(y2, x2, p);
                value.at(direction)--;

                if (value.at(direction) == 0)
                {
                    add_to_propagator(y2, x2, p);
                    wave.set(idx2, p, false);
                }
            }
        }
    }
}

void Propagator::init_compatible() noexcept
{
    std::array<int, 4> value;

    for (std::size_t y = 0; y < m_wave_height; y++)
    {
        for (std::size_t x = 0; x < m_wave_width; x++)
        {
            for (unsigned int pattern = 0; pattern < m_patterns_size;
                 pattern++)
            {
                for (unsigned int direction = 0; direction < 4; direction++)
                {
                    value.at(direction) = static_cast<unsigned int>(
                        m_propagator_state.at(pattern)
                            .at(get_opposite_direction(direction)).size());
                }
                m_compatible(y, x, pattern) = value;
            }
        }
    }
}
}  // namespace wfcpp

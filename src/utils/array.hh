// SPDX-License-Identifier: MIT

#ifndef UTILS_ARRAY_HH
#define UTILS_ARRAY_HH

#include <array>

namespace pixel_terrain {
    template <typename Tp, typename... Args>
    constexpr auto make_array(Args &&...args)
        -> std::array<Tp, sizeof...(Args)> {
        return std::array<Tp, sizeof...(Args)>{args...};
    }
} // namespace pixel_terrain

#endif

// SPDX-License-Identifier: MIT

#include <optional>
#include <type_traits>
#include <vector>

#include "nbt.hh"
#include "nbt/tag.hh"

namespace pixel_terrain::nbt {
    auto nbt::parse_buffer(std::vector<std::uint8_t>::const_iterator first,
                           std::vector<std::uint8_t>::const_iterator last)
        -> bool {
        auto [root, itr] = tag_compound::parse_buffer(first, last);
        if (root == nullptr || last < itr) {
            delete root;
            return false;
        }

        root_ = root;

        return true;
    }

    auto nbt::from_iterator(std::vector<std::uint8_t>::const_iterator first,
                            std::vector<std::uint8_t>::const_iterator last)
        -> nbt * {
        auto *result = new nbt;
        if (!result->parse_buffer(first, last)) {
            delete result;
            return nullptr;
        }

        return result;
    }
} // namespace pixel_terrain::nbt

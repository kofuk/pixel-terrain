// SPDX-License-Identifier: MIT

#ifndef NBT_HH
#define NBT_HH

#include <cstdint>
#include <type_traits>
#include <vector>

#include "nbt/nbt-path.hh"
#include "nbt/tag.hh"

namespace pixel_terrain::nbt {
    template <class Tp, class = std::is_convertible<Tp, tag_payload>>
    struct cxx_to_nbt_type {
        static auto type() -> tag_type { return tag_type::TAG_END; }
    };

    class nbt {
        tag_compound *root_ = nullptr;

        auto parse_buffer(std::vector<std::uint8_t>::const_iterator first,
                          std::vector<std::uint8_t>::const_iterator last)
            -> bool;

        nbt() = default;

    public:
        ~nbt() { delete root_; }

        static auto
        from_iterator(std::vector<std::uint8_t>::const_iterator first,
                      std::vector<std::uint8_t>::const_iterator last) -> nbt *;

        template <class Tp, class = std::is_convertible<Tp, tag_payload>>
        auto query(nbt_path path) -> Tp * {
            if (root_ == nullptr) {
                return nullptr;
            }

            tag_payload *result = root_->query(&path);

            if (result == nullptr ||
                cxx_to_nbt_type<Tp>::type() != result->type()) {
                return nullptr;
            }

            return static_cast<Tp *>(result);
        }
    };
} // namespace pixel_terrain::nbt

#endif

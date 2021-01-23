// SPDX-License-Identifier: MIT

#include <cctype>

#include "nbt-path.hh"

namespace pixel_terrain::nbt {
    namespace {
        using iterator = std::string::const_iterator;

        auto parse_tag_name(iterator first, iterator last, std::string *out)
            -> iterator {
            bool verbatim = false;
            for (; first != last; ++first) {
                if (verbatim) {
                    out->push_back(*first);
                    verbatim = false;
                } else {
                    if (*first == '\\') {
                        verbatim = true;
                    } else if (*first == '/' || *first == '[' || *first == '<') {
                        break;
                    } else {
                        out->push_back(*first);
                    }
                }
            }

            if (verbatim) {
                out->push_back('\\');
            }

            return first;
        }

        auto parse_index_number(iterator first, iterator last, std::size_t *out,
                                bool *ok) -> iterator {
            if (first == last) {
                *ok = false;
                return first;
            }

            *ok = true;
            iterator itr = first;

            *out = 0;
            if (*itr < '1' || '9' < *itr) {
                *ok = false;
                return first;
            }
            *out = *itr - '0';
            ++itr;

            for (; itr != last; ++itr) {
                if (!static_cast<bool>(std::isdigit(*itr))) {
                    break;
                }
                *out *= 10;
                *out += *itr - '0';
            }

            return itr;
        }

        auto parse_tag_compound_component(iterator first, iterator last,
                                          nbt_path::pathspec *spec, bool *ok)
            -> iterator {
            if (first == last || *first != '/') {
                *ok = false;
                return first;
            }

            std::string key;
            iterator itr = parse_tag_name(first + 1, last, &key);

            *spec = nbt_path::pathspec(key);
            *ok = true;

            return itr;
        }

        auto parse_array_index(iterator first, iterator last,
                         nbt_path::pathspec *spec, bool *ok) -> iterator {
            *ok = true;

            if (first == last) {
                *ok = false;
                return first;
            }

            iterator itr = first;

            if (*itr != '[') {
                *ok = false;
                return first;
            }
            ++itr;

            std::size_t idx;
            itr = parse_index_number(itr, last, &idx, ok);
            if (!*ok) {
                return first;
            }

            if (itr == last || *itr != ']') {
                *ok = false;
                return first;
            }
            ++itr;

            *spec = nbt_path::pathspec(idx, nbt_path::pathspec::container::ARRAY);

            return itr;
        }

        auto parse_list_index(iterator first, iterator last,
                         nbt_path::pathspec *spec, bool *ok) -> iterator {
            *ok = true;

            if (first == last) {
                *ok = false;
                return first;
            }

            iterator itr = first;

            if (*itr != '<') {
                *ok = false;
                return first;
            }
            ++itr;

            std::size_t idx;
            itr = parse_index_number(itr, last, &idx, ok);
            if (!*ok) {
                return first;
            }

            if (itr == last || *itr != '>') {
                *ok = false;
                return first;
            }
            ++itr;

            *spec = nbt_path::pathspec(idx, nbt_path::pathspec::container::LIST);

            return itr;
        }

        auto parse_path_compoent(iterator first, iterator last,
                                 nbt_path::pathspec *spec, bool *ok)
            -> iterator {
            if (first == last) {
                *ok = false;
                return first;
            }

            if (*first == '/') {
                iterator itr = parse_tag_compound_component(first, last, spec, ok);
                if (*ok) {
                    return itr;
                }
            } else if (*first == '[') {
                iterator itr = parse_array_index(first, last, spec, ok);
                if (*ok) {
                    return itr;
                }
            } else if (*first == '<') {
                iterator itr = parse_list_index(first, last, spec, ok);
                if (*ok) {
                    return itr;
                }
            }

            *ok = false;
            return first;
        }
    } // namespace

    auto nbt_path::compile(std::string const &path) -> nbt_path * {
        auto *result = new nbt_path;

        iterator itr = path.begin();
        iterator last = path.end();

        pathspec spec;
        bool ok;
        do {
            itr = parse_path_compoent(itr, last, &spec, &ok);
            if (ok) {
                result->path_.push_back(spec);
            }
        } while (ok);

        if (itr != last) {
            delete result;
            return nullptr;
        }

        return result;
    }
} // namespace pixel_terrain::nbt

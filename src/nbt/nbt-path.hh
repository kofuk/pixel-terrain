// SPDX-License-Identifier: MIT

#ifndef NBT_NBT_PATH_HH
#define NBT_NBT_PATH_HH

#include <list>
#include <memory>
#include <string>
#include <vector>

namespace pixel_terrain::nbt {
    class nbt_path {
    public:
        class pathspec {
        public:
            enum class category { INDEX, KEY };
            enum class container { NONE, ARRAY, LIST };

        private:
            category category_ = category::INDEX;
            container container_ = container::NONE;

            std::string key_;
            std::size_t index_;

        public:
            pathspec() = default;

            pathspec(pathspec const &another) = default;

            pathspec(pathspec &&another) {
                category_ = another.category_;
                container_ = another.container_;
                key_ = std::move(another.key_);
                index_ = another.index_;
            }

            pathspec(std::string const &key) {
                category_ = category::KEY;
                container_ = pathspec::container::NONE;
                key_ = key;
            }

            pathspec(std::size_t index, container cont) {
                category_ = category::INDEX;
                container_ = cont;
                index_ = index;
            }

            auto category() -> category const & { return category_; }

            auto container() -> container const & { return container_; }

            auto key() const -> std::string const & { return key_; }

            auto index() const -> std::size_t const & { return index_; }

            auto operator=(pathspec const &another) -> pathspec & {
                category_ = another.category_;
                container_ = another.container_;
                key_ = another.key_;
                index_ = another.index_;
                return *this;
            }

            auto operator=(pathspec &&another) -> pathspec & {
                category_ = another.category_;
                container_ = another.container_;
                key_ = std::move(another.key_);
                index_ = another.index_;
                return *this;
            }
        };

    private:
        std::list<pathspec> path_;
        bool valid_;

        /* Minecraft use empty list of TAG_End to represent
           empty TAG_List. It true, treat these as unset. */
        bool ignore_empty_list_;

        nbt_path() = default;

    public:
        nbt_path(nbt_path const &another)
            : path_(another.path_), valid_(another.valid_),
              ignore_empty_list_(another.ignore_empty_list_) {}

        nbt_path(nbt_path &&another)
            : path_(std::move(another.path_)), valid_(another.valid_),
              ignore_empty_list_(another.ignore_empty_list_) {}

        static auto compile(std::string const &path) -> nbt_path;

        auto path() const -> std::vector<pathspec> {
            std::vector<pathspec> result;
            for (pathspec const &spec : path_) {
                result.push_back(spec);
            }
            return result;
        }

        auto get_one() -> pathspec {
            pathspec result = path_.front();
            path_.pop_front();

            return result;
        }

        auto remain() const -> bool { return !path_.empty(); }

        auto ignore_empty_list() const -> bool { return ignore_empty_list_; }

        auto set_ignore_empty_list(bool ignore = true) const -> nbt_path {
            nbt_path result(*this);
            result.ignore_empty_list_ = ignore;
            return result;
        }

        operator bool() const { return valid_; }

        auto operator!() const -> bool { return !valid_; }

        auto operator=(nbt_path const &another) -> nbt_path & {
            path_ = another.path_;
            valid_ = another.valid_;
            ignore_empty_list_ = another.ignore_empty_list_;
            return *this;
        }

        auto operator=(nbt_path &&another) -> nbt_path & {
            path_ = std::move(another.path_);
            valid_ = another.valid_;
            ignore_empty_list_ = another.ignore_empty_list_;
            return *this;
        }
    };
} // namespace pixel_terrain::nbt

#endif

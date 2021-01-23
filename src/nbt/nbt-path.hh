// SPDX-License-Identifier: MIT

#ifndef NBT_NBT_PATH_HH
#define NBT_NBT_PATH_HH

#include <list>
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

            auto key() const -> std::string const & {
                return key_;
            }

            auto index() const -> std::size_t const & {
                return index_;
            }

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

        nbt_path() = default;

    public:
        static auto compile(std::string const &path) -> nbt_path *;

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
    };
} // namespace pixel_terrain::nbt

#endif

#include <iostream>
#include <optional>
#include <string>
#include <map>

template <typename S, typename V>
class trie {
public:
    void add(const S& sequence, const V& value)
    {
        add_impl(sequence, value);
    }

    bool find(const S& sequence, V& value) const
    {
        return find_impl(sequence, value);
    }

private:
    struct node {
        std::map<typename S::value_type, node> to_;
        std::optional<V> value_;
    };

    void add_impl(const S& sequence, const V& value)
    {
        node* cur = &root_;
        for (const auto& unit: sequence) {
            auto it = cur->to_.find(unit);
            if (it == cur->to_.end()) {
                auto p = cur->to_.insert({unit, node()});
                it = p.first;
            }

            cur = &it->second;
        }

        cur->value_ = value;
    }

    bool find_impl(const S& sequence, V& value) const
    {
        const node* cur = &root_;
        for (const auto& unit: sequence) {
            auto it = cur->to_.find(unit);
            if (it == cur->to_.end())
                return false;

            cur = &it->second;
        }

        if (!cur->value_)
            return false;

        value = *(cur->value_);
        return true;
    }

private:
    node root_;
};

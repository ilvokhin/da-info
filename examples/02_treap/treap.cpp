#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <random>
#include <memory>
#include <limits>
#include <cmath>

// There are at least two known ways to implement treap:
// 1. through operations split and merge,
// 2. with rotations.
// Usually I prefer first one, it is more generic and simple, but this
// implementation uses rotations. Because rotations are important for AVL
// and RB trees implementations.
template <typename K, typename V>
class treap {
protected:
    struct node;
    // this should be std::unique_ptr, but code will be more complex for
    // understanding in this case
    using node_ptr = std::shared_ptr<node>;

public:
    treap():
        random_generator_(random_device_()),
        distribution_(0, std::numeric_limits<int>::max())
    {
    }

    bool find(const K& key, V& value) const
    {
        node_ptr cur = root_;

        while (cur) {
            if (cur->key_ == key) {
                value = cur->value_;
                return true;
            }

            if (key < cur->key_)
                cur = cur->left_;
            else
                cur = cur->right_;
        }

        return false;
    }

    void insert(const K& key, const V& value)
    {
        auto rookie = std::make_shared<node>(key, value, get_priority());
        root_ = insert_impl(root_, rookie);
    }

    void erase(const K& key)
    {
        root_ = erase_impl(root_, key);
    }

protected:
    struct node {
        node(const K& key, const V& value, int priority):
            key_(key), value_(value), priority_(priority)
        { }

        K key_;
        V value_;

        int priority_;
        node_ptr left_;
        node_ptr right_;
    };

    node_ptr rotate_left(node_ptr& root)
    {
        node_ptr next_root = root->right_;
        if (!next_root)
            return root;

        root->right_ = next_root->left_;
        next_root->left_ = root;

        return next_root;
    }

    node_ptr rotate_right(node_ptr& root)
    {
        node_ptr next_root = root->left_;
        if (!next_root)
            return root;

        root->left_ = next_root->right_;
        next_root->right_ = root;

        return next_root;
    }

    node_ptr insert_impl(node_ptr& root, node_ptr& rookie)
    {
        if (!root)
            return rookie;

        if (root->key_ == rookie->key_) {
            root->value_ = rookie->value_;
            return root;
        }

        if (rookie->key_ < root->key_) {
            root->left_ = insert_impl(root->left_, rookie);
            if (root->left_->priority_ > root->priority_)
                root = rotate_right(root);
        } else {
            root->right_ = insert_impl(root->right_, rookie);
            if (root->right_->priority_ > root->priority_)
                root = rotate_left(root);
        }

        return root;
    }

    node_ptr erase_impl(node_ptr& root, const K& key)
    {
        if (!root)
            return root;

        if (key != root->key_) {
            if (key < root->key_)
                root->left_ = erase_impl(root->left_, key);
            else if (key > root->key_)
                root->right_ = erase_impl(root->right_, key);
            return root;
        }

        if (!root->left_)
            return root->right_;

        if (!root->right_)
            return root->left_;

        if (root->left_->priority_ > root->right_->priority_) {
            root = rotate_right(root);
            root->right_ = erase_impl(root->right_, key);
        } else {
            root = rotate_left(root);
            root->left_ = erase_impl(root->left_, key);
        }

        return root;
    }

    int get_priority()
    {
        return distribution_(random_generator_);
    }

    node_ptr root_;

    std::random_device random_device_;
    std::mt19937 random_generator_;
    std::uniform_int_distribution<std::mt19937::result_type> distribution_;
};

using ull = unsigned long long;
class serializable_treap: public treap<std::string, ull>
{
public:
    void serialize(const std::string& filename) const
    {
        std::ofstream os(filename);

        serialize_impl(root_, os);
    }

    void deserialize(const std::string& filename)
    {
        std::ifstream is(filename);

        root_ = deserialize_impl(is);
    }

private:
    // you need to reimplement this method, everything should be written in
    // binary mode
    void serialize_impl(const node_ptr& root, std::ofstream& os) const
    {
        if (!root)
            return;

        // think carefully how you should write string in binary in correct way
        os << root->key_ << '\t';
        os << root->value_ << '\t';
        os << root->priority_ << '\t';

        bool has_left = (root->left_ != nullptr);
        bool has_right = (root->right_ != nullptr);

        os << has_left << '\t';
        os << has_right << '\n';

        if (has_left)
            serialize_impl(root->left_, os);

        if (has_right)
            serialize_impl(root->right_, os);
    }

    // this method have to be reimplemented too
    node_ptr deserialize_impl(std::ifstream& is)
    {
        node_ptr root;

        std::string key;
        // in perfect world we have all or nothing
        if (!(is >> key))
            return root;

        ull value = 0;
        is >> value;

        int priority = 0;
        is >> priority;

        bool has_left = false;
        bool has_right = false;

        is >> has_left;
        is >> has_right;

        root = std::make_shared<node>(key, value, priority);

        if (has_left)
            root->left_ = deserialize_impl(is);

        if (has_right)
            root->right_ = deserialize_impl(is);

        return root;
    }
};

void insert(serializable_treap& t)
{
    std::string key;
    ull value = 0;

    std::cin >> key >> value;
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    if (t.find(key, value)) {
        std::cout << "Exist\n";
    } else {
        t.insert(key, value);
        std::cout << "OK\n";
    }
}

void remove(serializable_treap& t)
{
    std::string key;
    ull value = 0;

    std::cin >> key;
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    if (!t.find(key, value)) {
        std::cout << "NoSuchWord\n";
    } else {
        t.erase(key);
        std::cout << "OK\n";
    }
}

void save_load(serializable_treap& t)
{
    std::string subcommand;
    std::string filename;

    std::cin >> subcommand >> filename;

    if (subcommand[0] == 'S')
        t.serialize(filename);
    else
        t.deserialize(filename);
    
    std::cout << "OK\n";
}

void check(const serializable_treap& t, const std::string& raw_key)
{
    std::string key(raw_key);
    ull value = 0;

    std::transform(key.begin(), key.end(), key.begin(), ::tolower);

    if (t.find(key, value))
        std::cout << "OK: " << value << '\n';
    else
        std::cout << "NoSuchWord\n";
}

int main()
{
    std::ios::sync_with_stdio(false);

    std::string cmd;
    serializable_treap t;

    while (std::cin >> cmd) {
        switch (cmd[0]) {
            case '+':
                insert(t);
                break;
            case '-':
                remove(t);
                break;
            case '!':
                save_load(t);
                break;
            default:
                check(t, cmd);
                break;
        }
    }

    return 0;
}

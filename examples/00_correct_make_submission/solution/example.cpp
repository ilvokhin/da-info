#include <algorithm>
#include <iterator>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <utility>
#include <string>
#include <vector>
#include <limits>
#include <queue>
#include <cmath>
#include <map>
#include <set>

using namespace std;

using ll = long long;

struct kv {
    int key;
    string value;
};

struct less_by_key {
    bool operator()(const kv& x, const kv& y) {
        return x.key < y.key;
    }
};

vector<kv> counting_sort(const vector<kv>& seq)
{
    vector<kv> out;
    auto mx = max_element(seq.begin(), seq.end(), less_by_key());
    if (mx == seq.end())
        return out;

    vector<int> cnt(mx->key + 1);

    for (const auto& elem: seq)
        ++cnt[elem.key];

    for (size_t i = 1; i < cnt.size(); ++i)
        cnt[i] += cnt[i - 1];

    int n = seq.size();
    out.resize(n);
    for (int i = n - 1; i >= 0; --i) {
        int& pos = cnt[seq[i].key];
        --pos;
        out[pos] = seq[i];
    }

    return out;
}

int main()
{
    std::ios::sync_with_stdio(false);

    vector<kv> elems;
    kv val;

    while (cin >> val.key >> val.value)
        elems.push_back(val);

    const auto& out = counting_sort(elems);

    for (const auto& val: out)
        cout << val.key << '\t' << val.value << '\n';

    return 0;
}

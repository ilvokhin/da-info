#include <iostream>
#include <cassert>
#include <vector>

using namespace std;

int main()
{
    int n;
    cin >> n;

    assert(n > 1);

    vector<int> v;
    v.reserve(n);

    for (int i = 0; i < n; ++i) {
        int x = 0;
        cin >> x;
        v.push_back(x);
    }

    v.pop_back();

    for (int i = 0; i < n; ++i)
        cout << v[i] << endl;

    return 0;
}

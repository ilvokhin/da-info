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

    cout << v[0] << endl;

    return 0;
}

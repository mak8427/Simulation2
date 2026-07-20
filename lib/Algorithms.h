//
// Created by dadoi on 21/06/2022.
//
// Utility functions used by the simulation.
//
#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

// In-place descending sort on a column of a vector-of-vectors.
// Each inner vector has the form {pop_index, bid_price, quantity}.
// O(n log n) via std::sort.
inline vector<vector<float>>& SortBids(vector<vector<float>>& m, int column, int length = -1) {
    if (length < 0) length = static_cast<int>(m.size());
    std::sort(m.begin(), m.begin() + length,
        [column](const vector<float>& a, const vector<float>& b) {
            return a[column] > b[column];
        });
    return m;
}

// Legacy wrapper kept for compatibility — sorts entire matrix by column descending.
inline auto Sort_Vector(vector<vector<float>> m, int column, int length, bool test) {
    return SortBids(m, column);
}

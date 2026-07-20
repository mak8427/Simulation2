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

// In-place descending selection sort on a column of a vector-of-vectors.
// Each inner vector has the form {pop_index, bid_price, quantity}.
// Sorts rows [0, length) by `column` in descending order.
// Returns a reference to the input for chaining.
inline vector<vector<float>>& SortBids(vector<vector<float>>& m, int column, int length = -1) {
    if (length < 0) length = static_cast<int>(m.size());
    for (int l = 0; l < length; l++) {
        int best = l;
        for (int i = l + 1; i < length; i++) {
            if (m[i][column] > m[best][column]) {
                best = i;
            }
        }
        if (best != l) {
            swap(m[l], m[best]);
        }
    }
    return m;
}

// Legacy wrapper kept for compatibility — sorts entire matrix by column descending.
inline auto Sort_Vector(vector<vector<float>> m, int column, int length, bool test) {
    return SortBids(m, column);
}

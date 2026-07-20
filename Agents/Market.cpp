//
// Created by dadoi on 25/04/2022.
//
// Posted-price market clearing:
//   Firms set prices (markup over unit cost), consumers buy cheapest-first.
//   Market price = volume-weighted average transaction price.
//   Grounded in:
//     Deissenberg et al. (2008) EURACE model — posted-price with inventory feedback.
//     Sutton (1991) Sunk Costs and Market Structure — markup pricing in competition.
//
#include <math.h>
#include "../lib/Algorithms.h"
#include "Market.h"
#include "Factory.h"
#include "Pop.h"
#include "Market.h"
#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <stdio.h>

using namespace std;

// Legacy — kept for source compatibility, unused in new architecture
void Market::value_change(){}
void Market::Sender(){}
void Market::Sender_cloth(){}

// Posted-price market clearing.
// 1. Sort factories by price (cheapest first).
// 2. For each pop (sorted by Money_for_X, highest first), buy from cheapest factory.
// 3. Market Stats["_value"] = volume-weighted average transaction price.
void Market::ClearMarket(string good_type){
    if (Pops == nullptr || Factories == nullptr) return;
    float pre_good = Stats[good_type + "_value"];
    double total_revenue = 0;
    float total_consumed = 0;
    string bid_key = (good_type == "Food") ? "Money_for_Food" : "Money_for_Cloth";

    // Collect bids
    vector<array<float,3>> bids;
    bids.reserve(Pops->size());
    for (int i = 0; i < Pops->size(); i++) {
        float demand = (good_type == "Food")
            ? Pops->at(i).food_consumed()
            : Pops->at(i).cloth_used();
        bids.push_back({float(i), Pops->at(i).Consumption[bid_key], demand});
    }
    std::sort(bids.begin(), bids.end(),
        [](const array<float,3>& a, const array<float,3>& b) { return a[1] > b[1]; });

    // Collect factory offers: {factory_idx, price, remaining_stock}
    vector<array<float,3>> offers;
    offers.reserve(Factories->size());
    for (int f = 0; f < Factories->size(); f++) {
        Factory* fac = (*Factories)[f];
        if (fac->type != good_type) continue;
        int stock = fac->production;  // what this factory produced this iteration
        if (stock <= 0) continue;
        offers.push_back({float(f), fac->my_price, float(stock)});
    }
    std::sort(offers.begin(), offers.end(),
        [](const array<float,3>& a, const array<float,3>& b) { return a[1] < b[1]; });

    // Match: highest-bidding pops buy from cheapest factory first
    for (int b = 0; b < bids.size(); b++) {
        int pop_idx = bids[b][0];
        float bid = bids[b][1];
        float demand = bids[b][2];
        if (demand <= 0 || bid <= 0) continue;

        for (int f = 0; f < offers.size() && demand > 0; f++) {
            if (offers[f][2] <= 0) continue;
            float price = offers[f][1];
            if (price > bid) continue;  // don't buy above willingness-to-pay
            float affordable = (price > 0)
                ? static_cast<float>(Pops->at(pop_idx).money / price)
                : demand;
            if (affordable <= 0) break;
            float bought = std::min(std::min(demand, offers[f][2]), affordable);

            double payment = double(price) * bought;
            Pops->at(pop_idx).money -= payment;
            total_revenue += payment;
            total_consumed += bought;
            offers[f][2] -= bought;
            demand -= bought;

            Factory* fac = (*Factories)[offers[f][0]];
            fac->cash += payment;       // direct money transfer
            fac->money_gained += payment;  // record for Mean_cash
            fac->stock_sold += bought;

            if (good_type == "Food") {
                Pops->at(pop_idx).Goods["Food"] += bought;
                if (pop_idx < food_bought.size()) food_bought[pop_idx] += bought;
            } else {
                Pops->at(pop_idx).Goods["Cloth"] += bought;
            }
        }
    }

    Stats[good_type + "_consumed"] += total_consumed;
    if (total_consumed > 0)
        Stats[good_type + "_value"] = total_revenue / total_consumed;
    if (good_type == "Food") {
        Stats["Food_Inflation"] = (Stats["Food_value"] / pre_good - 1);
        cout << " inflation: " << Stats["Food_Inflation"] << endl;
        cout << "price: " << Stats["Food_value"] << endl;
    } else {
        Stats["Cloth_Inflation"] = (Stats["Cloth_value"] / pre_good - 1);
        cout << "price: " << Stats["Cloth_value"] << endl;
    }
}

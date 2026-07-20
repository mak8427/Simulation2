//
// Created by dadoi on 25/04/2022.
//

#ifndef TEST1_MARKET_H
#define TEST1_MARKET_H
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "Factory.h"
#include "Pop.h"

using std::string;
using std::map;
class Pop;
class Factory;

class Market {
public:
    vector<vector<float>> food_market;
    std::vector<Pop>* Pops;
    std::vector<Factory*>* Factories;  // needed for posted-price clearing
    std::vector<string> Goods {"Food", "Cloth"};
    map<string, float> Stats = {
            {"Food_value", 100},
            {"Food_Inflation", 0},
            {"Food_produced", 0},
            {"Food_consumed", 0},
            {"Cloth_value", 100},
            {"Cloth_Inflation", 0},
            {"Cloth_produced", 0},
            {"Cloth_consumed", 0},
    };

    Market(std::vector<Pop>* insert_pop) {
        Pops = insert_pop;
        Factories = nullptr;
    }
    Market(std::vector<Pop>* insert_pop, std::vector<Factory*>* insert_fact) {
        Pops = insert_pop;
        Factories = insert_fact;
    }

    void Sender();           // legacy — kept for API compat
    void Sender_cloth();     // legacy
    void value_change();     // legacy
    void ClearMarket(string good_type);   // new posted-price clearing

    void Update(){
        Market::ClearMarket("Food");
        Market::ClearMarket("Cloth");
    }
};

#endif //TEST1_MARKET_H

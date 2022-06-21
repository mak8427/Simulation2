//
// Created by dadoi on 25/04/2022.
//

#ifndef TEST1_MARKET_H
#define TEST1_MARKET_H
#include <iostream>
#include <string>
#include <map>
#include <vector>

using std::string;
using std::map;

class Market {
public:
    std::vector<string> Goods {"Food", "Cloth"};
    map<string, float> Stats = { // This equals sign is optional
            {"Food_value", 100},
            {"Food_produced", 0},
            {"Food_consumed", 0},
            {"Cloth_value", 100},
            {"Cloth_produced", 0},
            {"Cloth_consumed", 0},
    };
    void  Food_value_change();
    void Update(){
        Market::Food_value_change();


    }

};


#endif //TEST1_MARKET_H

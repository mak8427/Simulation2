//
// Created by dadoi on 25/04/2022.
//

#ifndef TEST1_MARKET_H
#define TEST1_MARKET_H
#include <iostream>
#include <string>
#include <map>

using std::string;
using std::map;


class Market {
public:

    map<string, float> Stats = { // This equals sign is optional
            {"food_value", 10},
            {"food_produced", 0},
            {"food_consumed", 0},
            {"cloth_value", 5},
    };
    void  Food_value_change();
    void Update(){
        Market::Food_value_change();


    }

};


#endif //TEST1_MARKET_H

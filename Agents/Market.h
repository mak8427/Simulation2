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
class Pop ;

class Market {
public:
    std::vector<Pop>* Pops;
    std::vector<string> Goods {"Food", "Cloth"};
    map<string, float> Stats = { // This equals sign is optional
            {"Food_value", 100},
            {"Food_Inflation", 0},
            {"Food_produced", 0},
            {"Food_consumed", 0},
            {"Cloth_value", 100},
            {"Cloth_Inflation", 0},
            {"Cloth_produced", 0},
            {"Cloth_consumed", 0},
    };
    Market(std::vector<Pop>* insert_pop){
        Pops=insert_pop;
    }



    void Sender();
    void  value_change();
    void Sender_cloth();
    void Update(){
        Market::Sender();
        Market::Sender_cloth();

    }

};


#endif //TEST1_MARKET_H

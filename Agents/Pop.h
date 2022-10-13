//
// Created by dadoi on 22/04/2022.
//

#ifndef TEST1_POP_H
#define TEST1_POP_H

//
// Created by Mac on 15/04/22.
//

#include "../lib/json.hpp"
#include <iostream>
#include <stdio.h>      /* printf */
#include <math.h>
#include <chrono>
#include <thread>
#include <random>
#include <iomanip>
using namespace std;
class Factory;
class Market;
class Government;
class Pop {
public:
    map<string,float> Consumption{
            {"Food_supply",1},
            {"Food_importance",0.5},
            {"Cloth_importance", 0.5},
            {"Money_for_food",0},
            {"Money_for_cloth",0}
    };
    map<string,float>Goods{
            {"Food",0},
            {"Cloth",0}
    };
    int number=100;
    float money=50;
    float reproduction_rate;
    int months_with_food=0;
    nlohmann::json j=0;
    float reserve_constant=0;
    Factory* factory;
    Market* markets;
    Government* gov;
    void  Food_variation();
    void  Pop_variation();
    void  Money();
    float food_consumed();
    void SOL();
    void Migration();
    float cloth_used();
    void Update(){
        Pop::Food_variation();
        Pop::Money();
        Pop::Pop_variation();
        Pop::SOL();


    };
    Pop(){};
    Pop(int insert_number, int insert_money ,int insert_food, Factory* insert_factory,Market* insert_Market,Government* insert_gov,nlohmann::json insert_j){
        number=insert_number;
        money=insert_money;
        Goods["Food"]=insert_food;
        factory = insert_factory;
        markets=insert_Market;
        gov=insert_gov;
        j=insert_j;
        reserve_constant=j["reserve_constant"];
        reproduction_rate=j["reproduction_rate"];



    }

};


#endif //TEST1_POP_H


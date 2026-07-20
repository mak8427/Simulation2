//
// Created by dadoi on 25/04/2022.
//
#include <math.h>
#include "../lib/Algorithms.h"
#include "Market.h"
#include "Factory.h"
#include "Pop.h"
#include "Market.h"
#include <vector>
#include <iostream>
#include <stdio.h>


using namespace std;
void Market::value_change(){
    for (auto & good : Goods){
         if (Stats[good+"_produced"]>Stats[good+"_consumed"]) {
             float rect=float(Stats[good+"_consumed"])/float(Stats[good+"_produced"])-1;
            Stats[good+"_value"]=Stats[good+"_value"]+rect*log(std::max(1.0f, Stats[good+"_value"]));
         }
         else if (Stats[good+"_produced"] == 0 && Stats[good+"_consumed"] == 0){
             continue;
         }
         else{
             float rect=(1-1/(float(Stats[good+"_produced"])/float(Stats[good+"_consumed"])))/5;
             Stats[good+"_value"]=Stats[good+"_value"]*(1-rect);

         }
    }
}
void Market::Sender() {
    float pre_good=Stats["Food_value"];
    int i=0;

    vector<vector<float>> Price_for_goods;
    for(auto  &pop_z: *Pops){
        Price_for_goods.push_back({float(i),pop_z.Consumption["Money_for_Food"],pop_z.food_consumed()});
        i++;
    }
    Price_for_goods = Sort_Vector(Price_for_goods,1,Price_for_goods.size(),false);

    int foods = Stats["Food_produced"];
    float price = Stats["Food_value"];
    float consumed = 0;
    for(int i =0; i<Price_for_goods.size();i++){
        int idx = Price_for_goods[i][0];
        float demand = Pops->at(idx).food_consumed();
        if (foods >= demand){
            foods -= demand;
            consumed += demand;
            price = Price_for_goods[i][1];
            Pops->at(idx).Goods["Food"] += demand;
            Pops->at(idx).money -= price * demand;
        }
        else{
            consumed += foods;
            price = Price_for_goods[i][1];
            Pops->at(idx).Goods["Food"] += foods;
            Pops->at(idx).money -= price * foods;
            break;
        }
    }
    Stats["Food_consumed"] += consumed;
    Stats["Food_value"] = price;
    Stats["Food_Inflation"] = (Stats["Food_value"]/pre_good - 1);
    cout<<" inflation: "<<Stats["Food_Inflation"]<<endl;

    cout<<"price: "<<price<<endl;
    food_market = Price_for_goods;
}
void Market::Sender_cloth() {
    float pre_good=Stats["Cloth_value"];
    int i=0;
    vector<vector<float>> Price_for_goods;
    for(auto  &pop_z: *Pops){
        Price_for_goods.push_back({float(i),pop_z.Consumption["Money_for_Cloth"],pop_z.cloth_used()});
        i++;
    }
    Price_for_goods = Sort_Vector(Price_for_goods,1,Price_for_goods.size(),false);

    int cloths = Stats["Cloth_produced"];
    float price = Stats["Cloth_value"];
    float consumed = 0;
    for(int i =0; i<Price_for_goods.size();i++){
        int idx = Price_for_goods[i][0];
        float demand = Pops->at(idx).cloth_used();
        if (cloths >= demand){
            cloths -= demand;
            consumed += demand;
            price = Price_for_goods[i][1];
            Pops->at(idx).Goods["Cloth"] += demand;
            Pops->at(idx).money -= price * demand;
        }
        else{
            consumed += cloths;
            price = Price_for_goods[i][1];
            Pops->at(idx).Goods["Cloth"] += cloths;
            Pops->at(idx).money -= price * cloths;
            break;
        }
    }
    Stats["Cloth_consumed"] += consumed;
    Stats["Cloth_value"] = price;
    Stats["Cloth_Inflation"] = (Stats["Cloth_value"]/pre_good - 1);
    cout<<"price: "<<price<<endl;
}



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
            Stats[good+"_value"]=Stats[good+"_value"]+rect*log(Stats[good+"_value"]);
         }
         else{
             float rect=(1-1/(float(Stats[good+"_produced"])/float(Stats[good+"_consumed"])))/5;
             Stats[good+"_value"]=Stats[good+"_value"]*(1-rect);

         }
    }
}
void Market::Sender() {
    bool DEBUG_1=false;
    float pre_good=Stats["Food_value"];
    float Price_per_good[Pops->size()][3];
    int i=0;

    i=0;
    vector<vector<float>> Price_for_goods;
    for(auto  &pop_z: *Pops){

        Price_for_goods.push_back({float(i),pop_z.factory->salary*pop_z.Consumption["Food_importance"] / pop_z.food_consumed(),pop_z.food_consumed()});
        i++;
    }
    Price_for_goods = Sort_Vector(Price_for_goods,1,Price_for_goods.size(),false);
    if(DEBUG_1==true){
    for (int x = 0; x < Price_for_goods.size(); x++) {
        std::cout << Price_for_goods[x][0] << "  " << Price_for_goods[x][1] << "  " << Price_for_goods[x][2] << '\n';
    }
    }
    int foods =Stats["Food_produced"];
    float price;
    for(int i =0; i<Price_for_goods.size();i++){
        if (foods-Price_for_goods[i][2]>=0){
            foods-=Price_for_goods[i][2];
            Stats["Food_consumed"]+=Price_for_goods[i][2];
            price=Price_for_goods[i][1];
            Pops->at(Price_for_goods[i][0]).Goods["Food"] += Pops->at(Price_for_goods[i][0]).food_consumed();
        }
        else{
            Stats["Food_consumed"]+=Price_for_goods[i][2];
            price=Price_for_goods[i][1];
            Pops->at(Price_for_goods[i][0]).Goods["Food"] += foods;
            break;
        }
    }
    Stats["Food_value"]=price;
    for (int i=0; i<Price_for_goods.size();i++){
        if (Price_per_good[i][1]>=price) {
            Pops->at(Price_for_goods[i][0]).money -= price * Pops->at(Price_for_goods[i][0]).food_consumed();
        }
    }
    Stats["Food_Inflation"]=(Stats["Food_value"]/pre_good-1);
    cout<<" inflation: "<<Stats["Food_Inflation"]<<endl;

    cout<<"price: "<<price<<endl;
    food_market=Price_for_goods;
}
void Market::Sender_cloth() {
    float pre_good=Stats["Cloth_value"];
    float Price_per_good[Pops->size()][3];
    int i=0;
    vector<vector<float>> Price_for_goods;
    for(auto  &pop_z: *Pops){
        Price_for_goods.push_back({float(i),pop_z.factory->salary*(1-pop_z.Consumption["Food_importance"])/ pop_z.cloth_used(),pop_z.cloth_used()});
        i++;
    }
    Price_for_goods = Sort_Vector(Price_for_goods,1,Price_for_goods.size(),false);

    for (int x = 0; x < Price_for_goods.size(); x++) {
    /*std::cout << Price_for_goods[x][0] << "  " << Price_for_goods[x][1] << "  " << Price_for_goods[x][2] << '\n'; */
    }
    int cloths =Stats["Cloth_produced"];
    float price;
    for(int i =0; i<Price_for_goods.size();i++){
        if (cloths-Price_for_goods[i][2]>=0){
            cloths-=Price_for_goods[i][2];
            Stats["Cloth_consumed"]+=Price_for_goods[i][2];
            price=Price_for_goods[i][1];
            Pops->at(Price_for_goods[i][0]).Goods["Cloth"] += Pops->at(Price_for_goods[i][0]).cloth_used();
        }
        else{
            Stats["Cloth_consumed"]+=Price_for_goods[i][2];
            price=Price_for_goods[i][1];
            Pops->at(Price_for_goods[i][0]).Goods["Cloth"] += cloths;
            break;
        }
    }
    Stats["Cloth_value"]=price;

    for (int i=0; i<Price_for_goods.size();i++){
        if(Price_for_goods[i][1]>=price){
            Pops->at(Price_for_goods[i][0]).money-=price*Pops->at(Price_for_goods[i][0]).cloth_used();
        }
    }
    Stats["Cloth_Inflation"]=(Stats["Cloth_value"]/pre_good-1);
    cout<<"price: "<<price<<endl;
}



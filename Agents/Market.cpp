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
void Market::Food_value_change(){
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
    float Price_per_good[Pops->size()][3];
    int i=0;

    i=0;
    vector<vector<float>> Price_for_goods;
    for(auto  &pop_z: *Pops){
        Price_for_goods.push_back({float(i),pop_z.factory->salary/2 / pop_z.food_consumed(),pop_z.food_consumed()});
        i++;
    }
    Price_for_goods = Sort_Vector(Price_for_goods,1,Price_for_goods.size(),false);

    for (int x = 0; x < Price_for_goods.size(); x++) {
        std::cout << Price_for_goods[x][0] << "  " << Price_for_goods[x][1] << "  " << Price_for_goods[x][2] << '\n';
    }
    int foods =Stats["Food_produced"];
    cout<<foods<<"\n";
    float price;
    for(int i =0; i<Price_for_goods.size();i++){
        if (foods-Price_for_goods[i][2]>=0){
            foods-=Price_for_goods[i][2];
            Stats["Food_consumed"]+=Price_for_goods[i][2];
            price=Price_for_goods[i][1];
            Pops->at(Price_for_goods[i][0]).money-=price*Pops->at(Price_for_goods[i][0]).food_consumed();
            Pops->at(Price_for_goods[i][0]).food += Pops->at(Price_for_goods[i][0]).food_consumed();
        }
        else{
            Stats["Food_consumed"]+=Price_for_goods[i][2];
            price=Price_for_goods[i][1];
            Pops->at(Price_for_goods[i][0]).money-=price*foods;
            Pops->at(Price_for_goods[i][0]).food += foods;
            break;
        }
    }
    Stats["Food_value"]=price;
    cout<<"price: "<<price<<endl;

}
void Market::Sender_cloth() {
    float Price_per_good[Pops->size()][3];
    int i=0;
    vector<vector<float>> Price_for_goods;
    for(auto  &pop_z: *Pops){
        Price_for_goods.push_back({float(i),pop_z.factory->salary/2 / pop_z.cloth_used(),pop_z.cloth_used()});
        i++;
    }
    Price_for_goods = Sort_Vector(Price_for_goods,1,Price_for_goods.size(),false);

    for (int x = 0; x < Price_for_goods.size(); x++) {
        std::cout << Price_for_goods[x][0] << "  " << Price_for_goods[x][1] << "  " << Price_for_goods[x][2] << '\n';
    }
    int cloths =Stats["Cloth_produced"];
    cout<<cloths<<"\n";
    float price;
    for(int i =0; i<Price_for_goods.size();i++){
        if (cloths-Price_for_goods[i][2]>=0){
            cloths-=Price_for_goods[i][2];
            Stats["Cloth_consumed"]+=Price_for_goods[i][2];
            price=Price_for_goods[i][1];
            Pops->at(Price_for_goods[i][0]).money-=price*Pops->at(Price_for_goods[i][0]).cloth_used();
            Pops->at(Price_for_goods[i][0]).food += Pops->at(Price_for_goods[i][0]).cloth_used();
        }
        else{
            Stats["Cloth_consumed"]+=Price_for_goods[i][2];
            price=Price_for_goods[i][1];
            Pops->at(Price_for_goods[i][0]).money-=price*cloths;
            Pops->at(Price_for_goods[i][0]).cloth += cloths;
            break;
        }
    }
    Stats["Cloth_value"]=price;
    cout<<"price: "<<price<<endl;

}



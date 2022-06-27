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
         if (Stats[good+"_produced"]<Stats[good+"_consumed"]) {
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
    for(auto &pop_z : *Pops)
    {
        Price_per_good[i][0] = i;
        Price_per_good[i][1] = pop_z.money / pop_z.food_consumed();
        Price_per_good[i][2] = pop_z.food_consumed();
        i++;
    }

    SortMatrix(Price_per_good,1);

}



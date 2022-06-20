//
// Created by dadoi on 25/04/2022.
//
#include <math.h>

#include "Market.h"
#include <iostream>
#include <stdio.h>
using namespace std;
void Market::Food_value_change(){
    if (Stats["food_produced"]<Stats["food_consumed"]) {
        float rect=float(Stats["food_consumed"])/float(Stats["food_produced"])-1;
       Stats["food_value"]=Stats["food_value"]+rect*log(Stats["food_value"]);
    }
    else{
        float rect=(1-1/(float(Stats["food_produced"])/float(Stats["food_consumed"])))/5;
        Stats["food_value"]=Stats["food_value"]*(1-rect);

    }



}
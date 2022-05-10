//
// Created by dadoi on 25/04/2022.
//
#include <math.h>

#include "Market.h"
#include <iostream>
#include <stdio.h>
using namespace std;
void Market::Food_value_change(){
    if (food_produced<food_consumed) {
        float rect=float(food_consumed)/float(food_produced)-1;
        food_value=food_value+rect*log(food_value);
    }
    else{
        float rect=(1-1/(float(food_produced)/float(food_consumed)))/5;
        food_value=food_value*(1-rect);

    }



}
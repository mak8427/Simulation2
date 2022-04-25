//
// Created by dadoi on 25/04/2022.
//

#include "Market.h"
#include <iostream>
#include <stdio.h>
using namespace std;
void Market::Food_value_change(){
    if (food_produced<food_consumed) {
        food_value=food_value+food_value*((food_consumed/food_produced)-1)+0.1;
    }
    else{
        food_value=food_value-food_value*(1/(food_produced/food_consumed));
    }



}
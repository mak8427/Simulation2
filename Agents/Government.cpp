//
// Created by dadoi on 04/05/2022.
//

#include "Government.h"
#include <math.h>
#include "Factory.h"
#include "Pop.h"
#include "Market.h"
#include <random>
#include <iostream>
#include <stdio.h>

using namespace std;

void Government::Census(){
    for (auto & i : *pop_) {
        n_tot=n_tot+i.number;
        n_food=n_food+i.food;

    }
    for (auto & i : *fact_) {
        gdp=gdp+i->production * market->Stats["Food_value"];
    }
}
void Government::Reset(){
    n_tot=0;
    gdp=0;
    n_food=0;
}

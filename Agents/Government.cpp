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

// Census: aggregate total population, food held, and GDP across all agents.
// GDP = sum over factories of production * market price for that good's type.
void Government::Census(){
    for (auto & i : *pop_) {
        n_tot=n_tot+i.number;
        n_food=n_food+i.Goods["Food"];
    }
    for (auto & i : *fact_) {
        gdp=gdp+i->production * market->Stats[i->type+"_value"];
    }
}

// Reset accumulators for the next iteration.
void Government::Reset(){
    n_tot=0;
    gdp=0;
    n_food=0;
}

void Government::Inflation(){
    float inflation=0;
    for (auto & i : *fact_) {
    }
}

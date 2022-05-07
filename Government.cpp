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
    }
}
void Government::Reset(){
    cout<<n_tot;
    n_tot=0;
}

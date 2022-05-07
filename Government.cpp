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
void Government::Taxation(){
    for (auto i = 0; i < pop_->size(); ++i) {
        cout<<pop_->at(i).number<<' ';
    }
}

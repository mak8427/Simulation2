//
// Created by dadoi on 22/04/2022.
//

#include "Factory.h"
#include "Pop.h"
#include "Good.h"

#include <iostream>
#include <stdio.h>

float Factory::Troughput(){
    float t=1;
    return t;
};
void Factory::Production(){
    int production=round(workers->number*Troughput());
    float money_gained=production*(good_type->value);

    cash=cash+money_gained;
};
void Factory::Salary(){
    salary=cash*0.9;

    cash=cash-salary;

};
float Factory::salary_(){
    return salary;


};
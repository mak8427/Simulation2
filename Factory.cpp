//
// Created by dadoi on 22/04/2022.
//
#include <math.h>
#include "Factory.h"
#include "Pop.h"
#include "Market.h"
#include <random>
#include <iostream>
#include <stdio.h>



float Factory::Troughput(){
    float t=(log(investment+10)/log(10000));
    return t;
};
void Factory::Production(){
    production=round(sqrt(workers->number)+cos(workers->number)+3.5-log(workers->number));
    float money_gained=production*(markets->food_value);
    markets->food_produced=markets->food_produced+production;
    cash=cash+money_gained;
};
void Factory::Salary(){
    salary=cash*1;
    cash=cash-salary;

};
float Factory::salary_(){
    return salary;


};
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

//test
float Factory::Troughput(){
    float t=(log(investment+10)/log(10000));
    return t;
};
void Factory::Production(){

    production= Efficiency_parameter* pow((distribution_parameter*pow(investment,sigma_sostitution)+(1-distribution_parameter)*pow(workers->number,sigma_sostitution)),1/sigma_sostitution);
    float money_gained=production*(markets->Stats["food_value"]);
    markets->Stats["food_produced"]=markets->Stats["food_produced"]+production;
    cash=cash+money_gained;
};
void Factory::Salary(){
    salary=cash*1;
    cash=cash-salary;

};
float Factory::salary_(){
    return salary;


};
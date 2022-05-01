//
// Created by dadoi on 22/04/2022.
//

#include "Factory.h"
#include "Pop.h"
#include "Market.h"

#include <iostream>
#include <stdio.h>

float Factory::Troughput(){
    float t=(log(investment+10)/log(10000));
    return 1;
};
void Factory::Production(){
    int production=round(workers->number*Troughput());
    float money_gained=production*(markets->food_value);
    markets->food_produced=markets->food_produced+production;
    cash=cash+money_gained;
};
void Factory::Salary(){
    float investment_add=cash*0.1;
    salary=cash*0.8;
    investment=investment_add+investment;

    cash=cash-salary-investment_add;

};
float Factory::salary_(){
    return salary;


};
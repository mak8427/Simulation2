//
// Created by dadoi on 22/04/2022.
//

#include "Factory.h"
#include "Pop.h"
#include "Good.h"



 float Factory::Troughput(){
    float t=1;
    return t;
};
void Factory::Production(){
    int production=round(workers->number*Troughput());
    float money_gained=production*(good_type->value);
    std::cout<<workers->number;
    cash=cash+money_gained;
};
void Factory::Salary(){
    salary=1000;
    cash=cash-salary;



};
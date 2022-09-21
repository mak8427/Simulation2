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
    money_gained=(production*markets->Stats[type+"_value"])/*(1/(1+markets->Stats[type+"_Inflation"]))*/;
    markets->Stats[type+"_produced"]=markets->Stats[type+"_produced"]+production;
    cash=cash+money_gained;
};
void Factory::Salary(){
    if (cash<0){
        cash=0;
    }
    float salary=Mean_cash_12_months();
    if (salary<0){
        salary=money_gained;
    }
    cash=cash-salary;

};
float Factory::salary_(){
    return salary;


};
float Factory::Mean_cash_12_months(){
    if (cash_12_months.size()<12)
    {
        cash_12_months.push_back(money_gained);
    }
    else{
        cash_12_months.erase(cash_12_months.begin());
        cash_12_months.push_back(money_gained);
    }
    float sum=0;
    for (int i=0;i<cash_12_months.size();i++){
        sum=sum+cash_12_months[i];
    }
    float mean=sum/cash_12_months.size();
    return mean;
}
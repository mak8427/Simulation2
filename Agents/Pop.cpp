//
// Created by dadoi on 22/04/2022.
//

#include "Pop.h"
#include "Factory.h"
#include "Market.h"
# include "Government.h"

float Pop::food_consumed() {
    int f = round(float(number));
    if (f == 0) {
        f = 1;
    }
    return f;
}


float Pop::cloth_used() {


    int f = number - Goods["Cloth"];
    if (f <= 0) {
        f = 1;
    }
    return f;
}

void Pop::Food_variation() {
    Goods["Food"] = Goods["Food"] - food_consumed();
    Goods["Cloth"] -= round(float(number) / 12);
};

void Pop::Pop_variation() {

    if (Goods["Food"] < 0) {
        float food_per_capita = float(food_consumed()) / float(number);
        int var = round(float(Goods["Food"]) / food_per_capita);
        number = number + var;
        /* number=number+1+number*reproduction_rate*(-food/food_consumed()); */

        if (number <= 0) {
            number = 1;
        }
    } else if (Goods["Food"] >= 0) {
        std::uniform_real_distribution<float> distr(0.05, 0.1);
        std::random_device rd;
        std::default_random_engine eng(rd());
        number = number + 1 + number * ((1 / (log(number + 0.1)) - 0.15) *(1 + distr(eng)));

    }
    if (Goods["Cloth"] <= 0) {

        Goods["Cloth"] = 0;
    }


};

void Pop::SOL() {
    std::random_device rd;
    std::default_random_engine eng(rd());
    // for good in Goods do good+1

    /*
float free_money=money;
map<string,float> Goods_demand{
        {"Food",food_consumed()},
        {"Cloth",cloth_used()}
};


for(auto &good : Goods){
    auto distr=Goods_distribution[good.first];
    if(good.second>0){
        Consumption[good.first+"_importance"]=markets->Stats[good.first+"_value"]*(1-distr(eng));

    }
    else if (good.second<0){
        Consumption[good.first+"_importance"]=markets->Stats[good.first+"_value"]*(1 + distr(eng));

    }
    if(free_money<Consumption[good.first+"_importance"]*Goods_demand[good.first]){
        Consumption[good.first+"_importance"]=free_money/Goods_demand[good.first];
        free_money=0;
    }
    else{
        free_money=free_money-Consumption[good.first+"_importance"]*Goods_demand[good.first];
    }

}
*/

    float Free_money_in_salary= money/factory->salary ;
    auto distr = std::uniform_real_distribution<float>(0.001, 0.1);
    float delta = Consumption["Food_importance"] * distr(eng);

    float personal_loan=0;
    if (Goods["Food"] > 0 ){
        months_with_food = months_with_food + 1;
        Consumption["Food_supply"] = 1;
        Consumption["Food_importance"] -= delta;

    } else if (Goods["Food"] < 0) {
        Consumption["Food_supply"] = Goods["Food"] / food_consumed();
        Consumption["Food_importance"] += delta;
        if (Consumption["Food_importance"] > 0.99 && money > personal_loan) {
            personal_loan = (Consumption["Food_importance"] - 0.99) * factory->salary;

        }
        if (Consumption["Food_importance"] > 0.99 && money < personal_loan) {
            cout<<"done!";
            Consumption["Food_importance"] = 0.98;
            personal_loan=money;
        }
        if (Consumption["Food_importance"] <=0.99) {
            personal_loan=0;
        }
        months_with_food = 0;
        Goods["Food"] = 0;
    } else if (Goods["Food"] == 0) {
        if (months_with_food >= 12) {
            months_with_food = months_with_food + 1;
            Consumption["Food_supply"] = 1;
        };
    }
    if ((1- Consumption["Food_importance"])<0.01){
        Consumption["Cloth_importance"]=0.01;
    }
    else{
        Consumption["Cloth_importance"]=1- Consumption["Food_importance"];
    }
    Consumption["Money_for_Food"]=(factory->salary*Consumption["Food_importance"]+ personal_loan) / food_consumed();
    Consumption["Money_for_Cloth"]=factory->salary*Consumption["Cloth_importance"] / cloth_used();
};

void Pop::Migration() {
    int a = 1;
};


void Pop::Money() {

    //salary from work
    float salary = factory->salary * (1 - gov->tax_rate);

    //tax payment
    gov->reserve = gov->reserve + gov->tax_rate * factory->salary;

    //real salary
    money = money + salary;
    /*if the pop could not afford enough food it uses all of it's money
    if  (food < food_consumed()*reserve_constant) {
        if (money <= (food_consumed() + 1) * markets->Stats["Food_value"]) {
            food = food + round(money / markets->Stats["Food_value"]);
            money = 0;
            months_with_food = 0;
        } else {
            food = food + food_consumed() + 1;
            money = money - (food_consumed() + 1) * markets->Stats["Food_value"];
            months_with_food = months_with_food + 1;
            cloth = cloth + round(money / markets->Stats["Cloth_value"]);
        }
    }
     */
    //markets->Stats["Cloth_consumed"]=markets->Stats["Cloth_consumed"]+4;

}

//
// Created by dadoi on 22/04/2022.
//

#include "Pop.h"
#include "Factory.h"
#include "Market.h"
# include "Government.h"

float Pop::food_consumed(){
    int f=round(float(number));
    if (f==0){
        f=1;
    }
    return f;
}


float Pop::cloth_used(){
    int f = round(float(number) / 10);
    if (f==0){
        f=1;
    }
    return f;
}
void Pop::Food_variation(){
    food=food-food_consumed();
    cloth=cloth-cloth_used();
};
void  Pop::Pop_variation(){

    if (food<0){
        float food_per_capita=float(food_consumed())/float(number);
        number=number+food/food_per_capita;
        /* number=number+1+number*reproduction_rate*(-food/food_consumed()); */
        number=
        food=0;
        if (number<=0){
            number=1;
        }
    }
    else if(food>=0){
        number=number+1+number*(1/(log(number+0.1))-0.15);
    }
    if (cloth<0){
        cloth=0;
    }




};
void Pop::SOL(){
    if(food>0){
        months_with_food = months_with_food + 1;
        Consumption["Food_supply"]=1;
        std::uniform_real_distribution<float> distr(0.001,0.05);
        std::random_device rd;
        std::default_random_engine eng(rd());
        Consumption["Food_importance"]=Consumption["Food_importance"]*(0.95+ distr(eng));

    }
    else if(food<0){
        Consumption["Food_supply"]= 1+food/food_consumed();
        std::uniform_real_distribution<float> distr(0.001,0.05);
        std::random_device rd;
        std::default_random_engine eng(rd());
        Consumption["Food_importance"]=Consumption["Food_importance"]*(1.01+distr(eng));
        if  (Consumption["Food_importance"]>0.99){
            Consumption["Food_importance"]=0.99;
        };
        months_with_food=0;
        food=0;
    }
    else if(food==0){
        if(months_with_food>=12){
            months_with_food = months_with_food + 1;
            Consumption["Food_supply"]=1;

        };
    }


    /*
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
};
void Pop::Migration(){
 int a=1;
};




void Pop::Money(){

    //salary from work
    float salary=factory->salary *(1-gov->tax_rate);

    //tax payment
    gov->reserve=gov->reserve+gov->tax_rate*factory->salary;

    //real salary
    money=money+salary;
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

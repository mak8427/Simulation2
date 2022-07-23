//
// Created by dadoi on 22/04/2022.
//

#include "Pop.h"
#include "Factory.h"
#include "Market.h"
# include "Government.h"
float Pop::food_consumed(){
    int f=round(sqrt(float(number)));
    return f;
}

float Pop::cloth_used(){
    int f=round(number/10);
    return f;
}
void Pop::Food_variation(){
    markets->Stats["Food_consumed"]=markets->Stats["Food_consumed"]+food_consumed();
    markets->Stats["Cloth_consumed"]=markets->Stats["Cloth_consumed"]+cloth_used();
    food=food-food_consumed();
    cloth=cloth-cloth_used();
};
void  Pop::Pop_variation(){

    if (food<0){
        float food_per_capita=float(food_consumed())/float(number);
        number=number+food/food_per_capita;
        food=0;
        if (number<=0){
            number=1;
        }

    }
    else if(months_with_food>=12){
        number=number+1+number*reproduction_rate;
    }



};
void Pop::SOL(){
    if(food>-30){
        months_with_food = months_with_food + 1;
    }
    else {
        months_with_food=0;
        food=0;
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
}







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

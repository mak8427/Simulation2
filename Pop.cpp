//
// Created by dadoi on 22/04/2022.
//

#include "Pop.h"
#include "Factory.h"
#include "Market.h"

float Pop::food_consumed(){
    int f=round(sqrt(float(number)));
    return f;
}
void Pop::Food_variation(){
    int food_c=food_consumed();
    food=food-food_c;
    markets->food_consumed=markets->food_consumed+food_c;
};
void  Pop::Pop_variation(){
    float food_per_capita=float(number)/float(food_consumed());
    number=number+food/food_per_capita;
    if (food<0){
        food=0;
    }



};

void Pop::Money(){
    money=money+factory->salary;
    food=food+round(money/markets->food_value);

    money=0;
    
}

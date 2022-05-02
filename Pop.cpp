//
// Created by dadoi on 22/04/2022.
//

#include "Pop.h"
#include "Factory.h"
#include "Market.h"


void Pop::Food_variation(){
    int food_c=round(sqrt(float(number)));
    food=food-food_c;
    markets->food_consumed=markets->food_consumed+food_c;
};
void  Pop::Pop_variation(){
    if (food<0){
        if (number<0){
            number=1;
            food=0;
        }
        else{
            number=number-1;
            food=0;
        }
    }
    else{
        number++;
    };

};

void Pop::Money(){
    money=money+factory->salary;
    food=food+round(money/markets->food_value);

    money=0;
    
}

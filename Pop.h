//
// Created by Mac on 15/04/22.
//

#ifndef TEST1_POP_H
#define TEST1_POP_H
#include <iostream>
#include <stdio.h>      /* printf */
#include <math.h>
#include <chrono>
#include <thread>
#include "Factory.h"
using namespace std;
class Factory;
class Pop {
public:
    int number=100;
    int money=50;
    int food=1000;
    Factory* factory;
    void Update(){
        Pop::Food_variation();
        Pop::Pop_variation();

    };
    Pop(int insert_number, int insert_money ,int insert_food, Factory* insert_factory){
        number=insert_number;
        money=insert_money;
        food=insert_food;
        factory=insert_factory;
    }
private:
    void Food_variation(){
        int food_p=number*(rand() % 3);
        int food_c=number*log(log(number+2));
        food=food+food_p-food_c;
        cout<<"Food: ";
        cout<<food<<std::endl;

    }
    void Pop_variation(){
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
        }

        cout<<"Number: ";
        cout<<number;
    }
};


#endif //TEST1_POP_H

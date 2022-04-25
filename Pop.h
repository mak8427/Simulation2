//
// Created by dadoi on 22/04/2022.
//

#ifndef TEST1_POP_H
#define TEST1_POP_H

//
// Created by Mac on 15/04/22.
//


#include <iostream>
#include <stdio.h>      /* printf */
#include <math.h>
#include <chrono>
#include <thread>

using namespace std;
class Factory;
class Good;
class Pop {
public:
    int number=100;
    float money=50;
    int food=1000;
    Factory* factory;
    void  Food_variation();
    void  Pop_variation();
    void  Money();
    void Update(){
        Pop::Food_variation();
        Pop::Pop_variation();
        Pop::Money();

    };
    Pop(int insert_number, int insert_money ,int insert_food, Factory* insert_factory){
        number=insert_number;
        money=insert_money;
        food=insert_food;
        factory = insert_factory;
    }

};


#endif //TEST1_POP_H


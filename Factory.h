//
// Created by Mac on 15/04/22.
//

#ifndef TEST1_FACTORY_H
#define TEST1_FACTORY_H
#include <string>
#include "good.h"
#include "Pop.h"
using namespace std;
class Pop;
class Factory{
public:
    float cash=1000;
    Pop* workers;
    Good* good_type;
    Factory(Pop* insert_pop, Good* insert_good){

        workers=insert_pop;
        good_type=insert_good;
    };
    void Update(){

    };
private:
    auto Troughput(){
        float t=1;
        return t;
    };
    auto Production(){
        int production=round(workers.number*Troughput());
        float money_gained=production*good_type.value;
        return money_gained;
    };

    auto Salary();


};
#endif //TEST1_FACTORY_H



#ifndef TEST1_FACTORY_H
#define TEST1_FACTORY_H

#include <string>

using namespace std;
class Pop;
class Market;
class Factory {
public:
    float investment=0;
    float cash = 1000;
    float salary=1000;
    float _ =5;
    Pop* workers;
    Market* markets;
    Factory(){};

    Factory(Pop* insert_pop, Market* insert_good){
        workers = insert_pop;
        markets = insert_good;
    };
    float Troughput();
    void Production();
    void Salary();
    float salary_();

    void Update(){
         Factory::Production();
         Factory::Salary();
    }


};
#endif //TEST1_FACTORY_H

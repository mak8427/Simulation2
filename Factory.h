

#ifndef TEST1_FACTORY_H
#define TEST1_FACTORY_H

#include <string>

using namespace std;
class Pop;
class Government;
class Market;
class Factory {
public:
    float investment=0;
    float cash = 1000;
    float salary=1000;
    float _ =5;
    Pop* workers;
    Government* gov;
    Market* markets;
    Factory(){};

    Factory(Pop* insert_pop, Market* insert_good,Government* insert_gov){
        workers = insert_pop;
        markets = insert_good;
        gov=insert_gov;
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

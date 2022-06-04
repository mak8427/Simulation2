

#ifndef TEST1_FACTORY_H
#define TEST1_FACTORY_H
#include "../lib/json.hpp"
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
    int production=0;
    float throughput=0;
    nlohmann::json j=0;
    Pop* workers;
    Government* gov;
    Market* markets;
    Factory(){};

    Factory(Pop* insert_pop, Market* insert_good,Government* insert_gov, nlohmann::json insert_j){
        workers = insert_pop;
        markets = insert_good;
        gov=insert_gov;
        j=insert_j;
        throughput= insert_j["Production_methods"]["PM_1"]["Throughput"];

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

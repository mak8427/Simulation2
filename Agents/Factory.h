

#ifndef TEST1_FACTORY_H
#define TEST1_FACTORY_H
#include "../lib/json.hpp"
#include <string>
#include <vector>

using namespace std;
class Pop;
class Government;
class Market;
class Factory {
public:
    vector<string> Goods {"Food", "Cloth"};
    vector<float> cash_12_months;
    string type;
    float investment=1;
    float cash = 1000;
    float money_gained=0;
    float sigma=0;
    float sigma_sostitution=0;
    float salary=1000;
    int production=0;
    float Efficiency_parameter=0;
    float distribution_parameter=0;
    nlohmann::json j=0;
    Pop* workers;
    Government* gov;
    Market* markets;
    Factory(){};

    Factory(Pop* insert_pop, Market* insert_good,Government* insert_gov, nlohmann::json insert_j, int random_int){
        //type=Goods[0];
        type=Goods[0];
        workers = insert_pop;
        markets = insert_good;
        gov=insert_gov;
        j=insert_j;
        Efficiency_parameter= insert_j[type]["Production_methods"]["PM_1"]["Efficiency_parameter"];
        sigma=insert_j[type]["Production_methods"]["PM_1"]["Sigma"];
        distribution_parameter=insert_j[type]["Production_methods"]["PM_1"]["distribution_parameter"];
        sigma_sostitution=(sigma-1)/sigma;


    };
    float Troughput();
    float Mean_cash_12_months();
    void Production();
    void Salary();
    float salary_();

    void Update(){
         Factory::Production();
         Factory::Salary();
    }


};
#endif //TEST1_FACTORY_H

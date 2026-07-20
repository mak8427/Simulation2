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
    vector<double> cash_12_months;
    int cash_ring_count = 0;
    int cash_ring_index = 0;
    double cash_ring_sum = 0;
    string type;
    float investment = 1;
    double cash = 1000;
    double money_gained = 0;
    float sigma = 0;
    float sigma_sostitution = 0;
    double salary = 1000;
    int production = 0;
    float Efficiency_parameter = 0;
    float distribution_parameter = 0;
    nlohmann::json j = 0;
    Pop* workers = nullptr;
    Government* gov = nullptr;
    Market* markets = nullptr;

    // Posted-price fields
    float my_price = 100;
    float my_markup = 0.10f;        // 10% initial
    float stock_sold = 0;           // filled by market each iteration
    float unit_cost = 0;
    int target_workers = 1;
    int worker_count = 1;
    int workforce = 0;

    Factory(){};
    Factory(Pop* insert_pop, Market* insert_good, Government* insert_gov, nlohmann::json insert_j, int random_int);

    float Troughput();
    double Mean_cash_12_months();
    void Production();
    void Salary();
    double salary_();
    void CalculatePrice();
    void AdjustWorkforce(vector<Pop>& pops, vector<Pop*>& unemployed);
    void Update();
};
#endif //TEST1_FACTORY_H

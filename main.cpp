#include <iostream>

#include <math.h>
#include <map>
#include <fstream>
#include <vector>
#include <chrono>
#include "lib/json.hpp"
#include <string>
#include "Agents/Factory.h"
#include "Agents/Market.h"
#include "Agents/Pop.h"
#include "Agents/Government.h"
using namespace nlohmann;
using namespace std;
int randoms(int y){
    int v= rand() % y + 1;
    return v;
};



int main() {

    // Iterations and time variables for the simulation to calculate the average time of every loop
    std::ios::sync_with_stdio(false);
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;


    // Initialize  json files
    std::ifstream production_methods("Production_methods.json");
    std::ifstream Pop_("Pop.json");
    std::ifstream Defines("Defines.json");
    json Pop_j = json::parse(Pop_);
    json production_methods_j = json::parse(production_methods);
    json Defines_j = json::parse(Defines);


    int n_agents = Defines_j["n_agents"];
    int ITERATIONS=Defines_j["ITERATIONS"];
    int STAT=Defines_j["STAT"];
    float TAX_RATE=Defines_j["TAX_RATE"];

    // Initialize the government, market , population and factories
    Market test_market;
    Government govs;
    vector<Factory *> factories;
    vector<Pop> pops;

    // Allocate the population and factories
    for (int i = 0; i <= n_agents; i++) {
        Factory *test_factory;
        factories.push_back(test_factory);
        pops.push_back(Pop(randoms(STAT), randoms(STAT), randoms(STAT), factories[i], &test_market, &govs, Pop_j["Pop"]));
    }
    for (int i = 0; i <= n_agents; i++) {
        factories[i] = new Factory(&pops[i], &test_market, &govs, production_methods_j["Food"]);
    }
    for (int i = 0; i <= n_agents; i++) {
        pops[i] = Pop(randoms(STAT), randoms(STAT), randoms(STAT), factories[i], &test_market, &govs, Pop_j["Pop"]);
    }

    // Initialize the Goverment
    govs=Government(&factories,&pops,TAX_RATE, &test_market);

    // Data dump
    ofstream myfile;
    myfile.open ("data.csv");

    // main Loop
    int n_cloth=0;
    for(int i=0; i<ITERATIONS; i++){
        auto t1 = high_resolution_clock::now();

        for (Factory*& entity : factories) {
            entity->Update();
        }
        n_cloth=0;
        for (Pop& entity : pops) {
            entity.Update();
            n_cloth=n_cloth+entity.cloth;
        }
        cout<<endl;
        test_market.Update();
        govs.Update();

        //Console dump
        cout<<"    Number of iterations: ";
        cout<<i<<std::endl;
        cout<<"food price: "<<test_market.food_value;
        cout<<"     tot numbers: "<<govs.n_tot;
        cout<<"     avg numbers: "<< govs.n_tot / n_agents;
        cout<<"     food consumed "<<test_market.food_consumed;
        cout<<"     cloth  "<<n_cloth/govs.n_tot<<'\n';
        cout<<"     gdp "<<govs.gdp;
        cout<<"     reserve "<<govs.reserve;
        cout<<"     food reserve "<<govs.n_food;
        cout<<"     food produced "<<test_market.food_produced<<'\n';

        //file dump
        myfile << i << ',' << govs.n_tot << ',' << govs.n_tot / n_agents << ',' << test_market.food_consumed << ',' << test_market.food_produced << ',' << test_market.food_value << ',' << govs.gdp << endl;


        //Reset of variables
        govs.Reset();
        test_market.food_consumed=0;
        test_market.food_produced=0;

        //time taken for each loop
        auto t2 = high_resolution_clock::now();
        cout<<"    Time: "<<duration_cast<std::chrono::microseconds>(t2-t1).count()<<" microseconds"<<std::endl;

    }

    myfile.close();
    getchar();
    return 0;
}




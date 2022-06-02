#include <iostream>
#include <stdio.h>      /* printf */
#include <math.h>
#include <map>
#include <fstream>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include <d3d9.h>
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
    std::ifstream f("../Production_methods.json");
    json j = json::parse(f);
    std::cout << j["Cloth"]["Production_methods"]["PM_1"] << std::endl;
    Market test_market;
    Government govs;
    //CREATOR AND ALLOCATOR
    int longs = 20;
    vector<Factory *> factories;
    vector<Pop> pops;
    for (int i = 0; i <= longs; i++) {
        Factory *test_factory;
        factories.push_back(test_factory);
        pops.push_back(Pop(randoms(300), randoms(300), randoms(300), factories[i], &test_market, &govs));
    }
    for (int i = 0; i <= longs; i++) {
        factories[i] = new Factory(&pops[i], &test_market, &govs);
    }
    for (int i = 0; i <= longs; i++) {
        pops[i] = Pop(randoms(300), randoms(300), randoms(300), factories[i], &test_market, &govs);
    }

    govs=Government(&factories,&pops,0, &test_market);




    int i=0;
    int n_cloth=0;
    ofstream myfile;
    myfile.open ("data.csv");
    while(i<100){

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
        i++;
        cout<<"    Number of iterations: ";
        cout<<i<<std::endl;
        cout<<"food price: "<<test_market.food_value;
        cout<<"     tot numbers: "<<govs.n_tot;
        cout<<"     avg numbers: "<<govs.n_tot/longs;
        cout<<"     food consumed "<<test_market.food_consumed;
        cout<<"     cloth  "<<n_cloth/govs.n_tot<<endl;
        cout<<"     gdp "<<govs.gdp;
        cout<<"     reserve "<<govs.reserve;
        cout<<"     food produced "<<test_market.food_produced<<std::endl;



        myfile<<i<<','<<govs.n_tot<<','<<govs.n_tot/longs<<','<<test_market.food_consumed<<','<<test_market.food_produced<<','<<test_market.food_value<<','<<govs.gdp<<endl;
        //this_thread::sleep_for(chrono::milliseconds(100) );
        test_market.food_consumed=0;
        test_market.food_produced=0;
        govs.Reset();


    }
    myfile.close();
    system("pause");
    return 0;
}



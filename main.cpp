#include <iostream>
#include <stdio.h>      /* printf */
#include <math.h>
#include <map>
#include <fstream>
#include <vector>
#include <fstream>
#include <chrono>
#include <thread>
#include "lib/json.hpp"
#include <string>
#include "Factory.h"
#include "Market.h"
#include "Pop.h"
#include "Government.h"
using namespace nlohmann;
using namespace std;
int randoms(){
    int v= rand() % 300 + 1;
    return v;
};

typedef struct{
    vector< Factory* > a;
    vector< Pop > b;
    Government c;
    Market d;
}ret_struct;
auto CREATOR(int longs){
    Market test_market;
    Government govs;
    vector< Factory* > factories;
    vector< Pop > pops;
    for(int i=0;i<=longs;i++) {
        Factory *test_factory;
        factories.push_back(test_factory);
        pops.push_back(Pop(randoms(), randoms(), randoms(), factories[i], &test_market,&govs));
    }
    for(int i=0;i<=longs;i++) {
        factories[i]=new  Factory (&pops[i],&test_market,&govs);
    }
    for(int i=0;i<=longs;i++) {
        pops[i]=Pop(randoms(), randoms(), randoms(), factories[i], &test_market,&govs);
    }

    ret_struct returned;
    returned.a=factories;
    returned.b= pops;
    returned.c =govs;
    returned.d = test_market;
    return returned ;




}


int main() {
    std::ifstream f("Production_methods.json");
    json j = json::parse(f);
    std::cout << j << std::endl;
    Market test_market;
    Government govs;
    //CREATOR AND ALLOCATOR
    int longs = 20;
    vector<Factory *> factories;
    vector<Pop> pops;
    for (int i = 0; i <= longs; i++) {
        Factory *test_factory;
        factories.push_back(test_factory);
        pops.push_back(Pop(randoms(), randoms(), randoms(), factories[i], &test_market, &govs));
    }
    for (int i = 0; i <= longs; i++) {
        factories[i] = new Factory(&pops[i], &test_market, &govs);
    }
    for (int i = 0; i <= longs; i++) {
        pops[i] = Pop(randoms(), randoms(), randoms(), factories[i], &test_market, &govs);
    }

    govs=Government(&factories,&pops,0.1, &test_market);




    int i=0;
    int n_cloth=0;
    ofstream myfile;
    myfile.open ("data.csv");
    while(i<3000){

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



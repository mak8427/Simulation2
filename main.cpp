#include <iostream>
#include <stdio.h>      /* printf */
#include <math.h>
#include <vector>
#include <fstream>

#include <chrono>
#include <thread>
#include <string>
#include "Factory.h"
#include "Market.h"
#include "Pop.h"
#include "Government.h"

using namespace std;
int randoms(){
    int v= rand() % 300 + 1;
    return v;
};

int main() {
    Market test_market;
    Government govs;
    int longs=20;
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
    govs=Government(&factories,&pops,0.1, &test_market);

    int i=0;
    int n_tot=0;
    int n_cloth=0;
    ofstream myfile;
    myfile.open ("data.csv");
    while(i<1000){

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



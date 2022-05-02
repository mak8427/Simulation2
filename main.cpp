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

using namespace std;
int randoms(){
    int v= rand() % 300 + 1;
    return v;
};
auto Vector_pop_factory(int longs, Market test_market) {
    vector< Factory* > factories;
    vector< Pop > pops;
    for(int i=0;i<=longs;i++) {
        Factory *test_factory;
        factories.push_back(test_factory);
        pops.push_back(Pop(randoms(), randoms(), randoms(), factories[i], &test_market));
    }
    for(int i=0;i<=longs;i++) {
        factories[i]=new  Factory (&pops[i],&test_market);
    }
    for(int i=0;i<=longs;i++) {
        pops[i]=Pop(randoms(), randoms(), randoms(), factories[i], &test_market);
    }
    struct retVals {        // Declare a local structure
        vector<Pop> i1;
        vector<Factory*> i2;
    };
    return retVals{pops,factories};
};

int main() {
    Market test_market;
    auto [pops_l,factories_l]  = Vector_pop_factory(5,test_market);
    int longs=1500;
    vector< Factory* > factories;
    vector< Pop > pops;
    for(int i=0;i<=longs;i++) {
        Factory *test_factory;
        factories.push_back(test_factory);
        pops.push_back(Pop(randoms(), randoms(), randoms(), factories[i], &test_market));
    }
    for(int i=0;i<=longs;i++) {
        factories[i]=new  Factory (&pops[i],&test_market);
    }
    for(int i=0;i<=longs;i++) {
        pops[i]=Pop(randoms(), randoms(), randoms(), factories[i], &test_market);
    }
    int i=0;
    int n_tot=0;
    ofstream myfile;
    myfile.open ("data.csv");
    while(i<3000){

        for (Factory*& entity : factories) {
            entity->Update();
        }
        n_tot=0;
        for (Pop& entity : pops) {
            entity.Update();
            n_tot=entity.number+n_tot;
        }
        cout<<endl;


        test_market.Update();
        i++;
        cout<<"    Number of iterations: ";
        cout<<i<<std::endl;
        cout<<"food price: "<<test_market.food_value;
        cout<<"     tot numbers: "<<n_tot;
        cout<<"     avg numbers: "<<n_tot/longs;
        cout<<"     food consumed "<<test_market.food_consumed;
        cout<<"     food produced "<<test_market.food_produced<<std::endl;



        myfile<<i<<','<<n_tot<<','<<n_tot/longs<<','<<test_market.food_consumed<<','<<test_market.food_produced<<','<<test_market.food_value<<endl;
        //this_thread::sleep_for(chrono::milliseconds(100) );
        test_market.food_consumed=0;
        test_market.food_produced=0;


    }
    myfile.close();
    system("pause");
    return 0;
}



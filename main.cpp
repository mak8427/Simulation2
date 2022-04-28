#include <iostream>
#include <stdio.h>      /* printf */
#include <math.h>
#include <vector>

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
    for(int i=0;i<=longs;i++){
        Factory* test_factory;
        Pop test_pop(randoms(),randoms(),randoms(),test_factory,&test_market);
        test_factory= new  Factory(&test_pop, &test_market);
        test_pop = Pop(randoms(),randoms(),randoms(),test_factory,&test_market);
        factories.push_back(new  Factory(&test_pop, &test_market));
        pops.push_back(Pop(randoms(),randoms(),randoms(),factories[i],&test_market));
        delete test_factory;
    }
    struct retVals {        // Declare a local structure
        vector<Pop> i1;
        vector<Factory*> i2;
    };
    return retVals{pops,factories};
};


int main() {
    Market test_market;
   auto [pops_l,factories_l]  = Vector_pop_factory(10,test_market);





    int i=0;
    while(i<1000000){
        for (Pop& entity : pops_l) {
            entity.Update();
        }
        cout<<endl;
        for (Factory*& entity : factories_l) {

            entity->Update();
        }
        test_market.Update();
        i++;
        cout<<"    Number of iterations: ";
        cout<<i<<std::endl;
        cout<<"food price: "<<test_market.food_value;
        cout<<"     food consumed "<<test_market.food_consumed;
        cout<<"     food produced "<<test_market.food_produced<<std::endl;

        test_market.food_consumed=0;
        test_market.food_produced=0;
        this_thread::sleep_for(chrono::milliseconds(100) );


    }

    system("pause");
    return 0;
}



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


int main() {
    std::vector< Factory* > arr;
    Factory* test_factory;
    Market test_market;
    Pop test_pop(100,100,100,test_factory,&test_market);
    test_factory= new  Factory(&test_pop, &test_market);
    test_pop = Pop(100,100,100,test_factory,&test_market);





    int i=0;
    while(i<1000000){
        test_factory->Update();
        test_pop.Update();
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



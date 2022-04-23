#include <iostream>
#include <stdio.h>      /* printf */
#include <math.h>
#include <chrono>
#include <thread>
#include <string>
#include "Factory.h"
#include "good.h"
#include "Pop.h"

using namespace std;


int main() {
    Factory* test_factory;
    Good test_good(1);
    Pop test_pop(100,100,100,test_factory);
    Factory c(&test_pop, &test_good);
    test_factory= &c;


    int i=0;
    float innovation=1.0;
    while(i<1000000){
        test_factory->Update();
        test_pop.Update();
        i++;
        innovation=sqrt(log(i));
        cout<<"    Number of iterations: ";
        cout<<i;
        cout<<"    Innovation: ";
        cout<<innovation<<std::endl;
        cout<<test_factory->cash<<std::endl;

        this_thread::sleep_for(chrono::milliseconds(100) );


    }

    system("pause");
    return 0;
}



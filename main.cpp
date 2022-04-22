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
    Good test(1);
    Factory factory1 = new Factory();
    Factory* puntatore=&factory1;
    Pop pop1(100,10,200,puntatore);
    Pop* puntatores = &pop1;
    factory1= factory1(puntatores,test);

    int i=0;
    float innovation=1.0;
    while(i<1000000){
        pop1.Update();
        i++;
        innovation=sqrt(log(i));
        cout<<"    Number of iterations: ";
        cout<<i;
        cout<<"    Innovation: ";
        cout<<innovation<<std::endl;
        this_thread::sleep_for(chrono::milliseconds(100) );


    }

    system("pause");
    return 0;
}



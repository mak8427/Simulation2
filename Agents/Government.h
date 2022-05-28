//
// Created by dadoi on 04/05/2022.
//

#ifndef TEST1_GOVERNMENT_H
#define TEST1_GOVERNMENT_H
#include <vector>
class Pop;
class Factory;
class Market;
using namespace std;

class Government {
public:
    float reserve=0;
    float tax_rate=0;
    float gdp=0;
    int n_tot=0;
    Market* market;
    vector< Factory*>* fact_;
    vector< Pop>* pop_;
    Government(vector<Factory*>* x ,vector<Pop>* y,float z, Market* q){
      fact_=x;
      pop_=y;
      tax_rate=z;
      market=q;
    }
    Government(){}
    void Update(){
        Census();
    };
    void Census();
    void Reset();

};


#endif //TEST1_GOVERNMENT_H

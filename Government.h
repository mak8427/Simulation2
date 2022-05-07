//
// Created by dadoi on 04/05/2022.
//

#ifndef TEST1_GOVERNMENT_H
#define TEST1_GOVERNMENT_H
#include <vector>
class Pop;
class Factory;

using namespace std;

class Government {
public:
    float reserve=0;
    float tax_rate=0.1;
    vector< Factory*>* fact_;
    vector< Pop>* pop_;
    Government(vector<Factory*>* x ,vector<Pop>* y,float z){
      fact_=x;
      pop_=y;
      tax_rate=z;
    }
    Government(){}
    void Update(){
        Taxation();
    };
    void Taxation();

};


#endif //TEST1_GOVERNMENT_H

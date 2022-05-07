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
    float Reserve=0;
    vector< Factory*>* fact_;
    vector< Pop>* pop_;
    Government(vector<Factory*>* x ,vector<Pop>* y){
      fact_=x;
      pop_=y;
    }
    Government(){}
    void Update(){
        Taxation();
    };
    void Taxation();

};


#endif //TEST1_GOVERNMENT_H

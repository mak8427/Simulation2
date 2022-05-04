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
  vector< Factory*>* fact;
  vector< Pop*>* pops;
    Government(vector< Factory*>* x ,vector< Pop*>* y){
      fact=x;
      pops=y;


  }


};


#endif //TEST1_GOVERNMENT_H

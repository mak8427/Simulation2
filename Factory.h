

#ifndef TEST1_FACTORY_H
#define TEST1_FACTORY_H

#include <string>

using namespace std;
class Pop;
class Good;
class Factory {
public:
    float cash = 1000;
    float salary=1000;
    Pop* workers;
    Good* good_type;

    Factory(Pop* insert_pop, Good* insert_good) {
        workers = insert_pop;
        good_type = insert_good;
    };
    void Update(){
        void Production();
        void Salary();
    }
    float Troughput();
    void Production();
    void Salary();

};
#endif //TEST1_FACTORY_H

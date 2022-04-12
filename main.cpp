#include <iostream>
#include <stdio.h>      /* printf */
#include <math.h>


class Pop {
public:
    int number=100;
    int money=50;
    int food=1000;
    void Variation(){
        int food_p=number*food*(1/number)*10;
        int food_c=number;
        food=food+food_p-food_c;
        std::cout<<food<<std::endl;

    }
    void Update(){
        Pop:Variation();
    }
};

int main() {
    Pop pop1;
    for(int i=0; i<20; i++){
        pop1.Update();
    }
    system("pause");
    return 0;
}



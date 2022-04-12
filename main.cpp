#include <iostream>
#include <stdio.h>      /* printf */
#include <math.h>
#include <Windows.h>


class Pop {
public:
    int number=100;
    int money=50;
    int food=1000;
    void Food_variation(){
        int food_p=number*(rand() % 3);
        int food_c=number*(rand()%3)*log((number*(rand()%2))+2)
                ;
        food=food+food_p-food_c;
        std::cout<<"Food: ";
        std::cout<<food<<std::endl;

    }
    void Pop_variation(){
        if (food<0){
            if (number<0){
                number=1;
                food=0;
            }
            else{
                number=number-1;
                food=0;
            }
        }
        else{
            number++;
        }
        std::cout<<"Number: ";
        std::cout<<number;
    }
    void Update(){
        Pop::Food_variation();
        Pop::Pop_variation();

    }
};

int main() {
    Pop pop1;
    int i=0;
    while(i<10000000000000){
        pop1.Update();
        i++;
        std::cout<<"    Number of iterations: ";
        std::cout<<i<<std::endl;
    }

    system("pause");
    return 0;
}



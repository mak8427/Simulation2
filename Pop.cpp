//
// Created by dadoi on 22/04/2022.
//

#include "Pop.h"
#include "Factory.h"

void Pop::Food_variation(){
    int food_c=number*log(log(number+2));
    food=food+-food_c;
    cout<<"Food: ";
    cout<<food<<std::endl;
};
void  Pop::Pop_variation(){
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
    };

    cout<<"Number: ";
    cout<<number;
};

void Pop::Money(){
    money=money+factory->salary;
    food=food+money*1;

}

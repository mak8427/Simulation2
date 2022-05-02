//
// Created by dadoi on 25/04/2022.
//

#ifndef TEST1_MARKET_H
#define TEST1_MARKET_H


class Market {
public:
    float food_value=10;
    float cloth_value=5;
    int food_produced=0;
    int food_consumed=0;
    void  Food_value_change();
    void Update(){
        Market::Food_value_change();

    }

};


#endif //TEST1_MARKET_H

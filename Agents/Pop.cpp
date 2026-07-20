//
// Created by dadoi on 22/04/2022.
//

#include "Pop.h"
#include "Factory.h"
#include "Market.h"
#include "Government.h"

// Each person needs exactly 1 unit of food per iteration.
// Clamp to 1 when population is zero to prevent division-by-zero downstream.
float Pop::food_consumed() {
    int f = round(float(number));
    if (f == 0) {
        f = 1;
    }
    return f;
}


// Cloth demand: each person uses ~1/12 unit per iteration.
// Returns the deficit (needed - held), or 0 if already stocked.
float Pop::cloth_used() {
    float needed = std::max(1.0f, round(float(number) / 12.0f));
    float deficit = needed - Goods["Cloth"];
    if (deficit > 0) {
        return deficit;
    }
    return 0;
}

// Consume food and cloth stockpiles each iteration.
void Pop::Food_variation() {
    Goods["Food"] = Goods["Food"] - food_consumed();
    Goods["Cloth"] -= round(float(number) / 12);
};

// Population dynamics:
//   - Starvation (food < 0): linear die-off proportional to food deficit.
//   - Growth (food > 0): logistically-limited growth with stochastic noise.
//     Formula: number += 1 + number * (1/log(number+0.1) - 0.15) * (1 + U(0.05, 0.1))
// Minimum population is clamped to 1.
void Pop::Pop_variation() {

    if (Goods["Food"] < 0) {
        float food_per_capita = float(food_consumed()) / float(number);
        int var = round(float(Goods["Food"]) / food_per_capita);
        number = number + var;
        /* number=number+1+number*reproduction_rate*(-food/food_consumed()); */

        if (number <= 0) {
            number = 1;
        }
    } else if (Goods["Food"] > 0) {
        std::uniform_real_distribution<float> distr(0.05, 0.1);
        std::random_device rd;
        std::default_random_engine eng(rd());
        number = number + 1 + number * ((1 / (log(number + 0.1)) - 0.15) *(1 + distr(eng)));

    }
    if (Goods["Cloth"] <= 0) {

        Goods["Cloth"] = 0;
    }


};

// Standard of Living: adjusts consumption priorities each iteration.
//   - Food_importance floats between 0.01 and 0.99 (self-correcting).
//   - When fed: Food_importance drifts down (prefer cloth/leisure).
//   - When starving: Food_importance rises, may trigger personal_loan.
//   - Unemployed pops bid from savings, not salary.
//   - Outputs: Money_for_Food and Money_for_Cloth (per-unit bid prices).
void Pop::SOL() {
    std::random_device rd;
    std::default_random_engine eng(rd());

    float worker_salary = (factory != nullptr) ? factory->salary : 0.0f;
    float Free_money_in_salary = (worker_salary > 0) ? money / worker_salary : 0.0f;
    auto distr = std::uniform_real_distribution<float>(0.001, 0.1);
    float delta = Consumption["Food_importance"] * distr(eng);

    personal_loan = 0;
    if (Goods["Food"] > 0 ){
        months_with_food = months_with_food + 1;
        Consumption["Food_supply"] = 1;
        Consumption["Food_importance"] -= delta;

    } else if (Goods["Food"] < 0) {
        Consumption["Food_supply"] = Goods["Food"] / food_consumed();
        Consumption["Food_importance"] += delta;
        if (Consumption["Food_importance"] > 0.99 && money > personal_loan) {
            personal_loan = (Consumption["Food_importance"] - 0.99) * factory->salary;

        }
        if (Consumption["Food_importance"] > 0.99 && money < personal_loan) {
            cout<<"done!";
            Consumption["Food_importance"] = 0.98;
            personal_loan=money;
        }
        if (Consumption["Food_importance"] <=0.99) {
            personal_loan=0;
        }
        months_with_food = 0;
        Goods["Food"] = 0;
    } else if (Goods["Food"] == 0) {
        if (months_with_food >= 12) {
            months_with_food = months_with_food + 1;
            Consumption["Food_supply"] = 1;
        };
    }
    if ((1- Consumption["Food_importance"])<0.01){
        Consumption["Cloth_importance"]=0.01;
    }
    else{
        Consumption["Cloth_importance"]=1- Consumption["Food_importance"];
    }
    // Total income-based budget: salary + 1/12 of savings as fallback
    float net_salary = (factory != nullptr) ? factory->salary * (1 - gov->tax_rate) : 0.0f;
    float savings_income = money / 12.0f;  // spend up to 1/12 of savings per iteration
    float bid_income = std::max(net_salary, savings_income);
    Consumption["Money_for_Food"]=(bid_income * Consumption["Food_importance"] + personal_loan) / food_consumed();
    float cloth_demand = cloth_used();
    if (cloth_demand > 0) {
        Consumption["Money_for_Cloth"]=net_salary*Consumption["Cloth_importance"] / cloth_demand;
    } else {
        Consumption["Money_for_Cloth"]=0;
    }
};

void Pop::Migration() {
    int a = 1;
};

// Labor mobility: workers switch factories based on wage differentials.
// Grounded in:
//   Farber (1999) Handbook of Labor Economics Ch.37 — monthly job-to-job
//     transition rate ~2%, elasticity to wage diff ~0.5.
//   Topel & Ward (1992) QJE — 10% avg wage gain from switching.
//   Burdett & Mortensen (1998) IER — equilibrium wage dispersion with OJS.
// P_switch = min(0.10, 0.02 + 0.5 * (target_salary - current_salary) / current_salary)
void Pop::TrySwitchJob(vector<Factory*>& factories, std::mt19937& gen) {
    if (factory == nullptr) return;  // unemployed don't switch
    if (factories.size() < 2) return;

    std::uniform_int_distribution<> factory_idx(0, factories.size() - 1);
    std::uniform_real_distribution<float> roll(0.0f, 1.0f);

    for (int tries = 0; tries < 3; tries++) {
        Factory* target = factories[factory_idx(gen)];
        if (target == nullptr || target == factory) continue;
        if (target->workers == nullptr) continue;

        float current_salary = factory->salary;
        if (current_salary <= 0) current_salary = 1.0f;
        float wage_diff = (target->salary - current_salary) / current_salary;

        if (wage_diff > 0) {
            float P_switch = std::min(0.10f, 0.02f + 0.5f * wage_diff);
            if (roll(gen) < P_switch) {
                factory = target;
                return;
            }
        }
    }
};


// Earn net salary (post-tax), transfer tax portion to government reserve.
// Unemployed pops earn nothing, consume savings.
void Pop::Money() {
    if (factory != nullptr) {
        float salary_net = factory->salary * (1 - gov->tax_rate);
        gov->reserve = gov->reserve + gov->tax_rate * factory->salary;
        money = money + salary_net;
    }
}

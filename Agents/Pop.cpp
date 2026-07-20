//
// Created by dadoi on 22/04/2022.
//

#include "Pop.h"
#include "Factory.h"
#include "Market.h"
#include "Government.h"
#include <algorithm>

// Each person needs exactly 1 unit of food per iteration.
float Pop::food_consumed() {
    return std::max(0, number);
}


// Cloth demand: each person uses ~1/12 unit per iteration.
// Returns the deficit (needed - held), or 0 if already stocked.
float Pop::cloth_used() {
    float needed = std::max(0.0f, round(float(number) / 12.0f));
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

// Infer local carrying capacity from the stock that existed immediately before
// Food_variation(). Main passes an explicit economy-wide production capacity.
void Pop::Pop_variation(std::mt19937& gen) {
    float carrying_capacity = std::max(0.0f, Goods["Food"] + food_consumed());
    Pop_variation(gen, carrying_capacity);
}

// Zulkarnaen & Rodrigo (2020), Eq. 2.1, using one simulation iteration as dt:
//   dN/dt = r*N*(1 - N/K(t))
// Food is measured in person-period units here, so the paper's linear model
// K(t) = alpha*I(t) has alpha=1. With K fixed during the iteration, the exact
// logistic step is K / (1 + (K/N - 1)*exp(-r)).
void Pop::Pop_variation(std::mt19937& gen, float carrying_capacity) {
    (void)gen;
    if (number <= 0) {
        number = 0;
        Goods["Food"] = 0;
        Goods["Cloth"] = 0;
        return;
    }

    if (reproduction_rate == 0) {
        if (Goods["Cloth"] <= 0) Goods["Cloth"] = 0;
        return;
    }

    if (carrying_capacity <= 0) {
        number = 0;
        Goods["Food"] = 0;
        Goods["Cloth"] = 0;
        return;
    }

    double population = number;
    double capacity = carrying_capacity;
    double denominator = 1.0
        + (capacity / population - 1.0) * std::exp(-reproduction_rate);
    number = static_cast<int>(std::lround(capacity / denominator));

    if (number <= 0) {
        number = 0;
        Goods["Food"] = 0;
        Goods["Cloth"] = 0;
        return;
    }
    if (Goods["Cloth"] <= 0) Goods["Cloth"] = 0;
};

// Apply the same equation to an aggregate economy without letting independent
// rounding of agent groups create a hidden minimum population. Fractional
// results are assigned by largest remainder so their sum equals the rounded
// aggregate result.
void Pop::Population_variation(std::vector<Pop>& pops,
                               std::mt19937& gen,
                               float total_carrying_capacity) {
    (void)gen;
    double total_population = 0;
    for (const Pop& pop : pops) total_population += std::max(0, pop.number);

    if (total_population <= 0) {
        for (Pop& pop : pops) pop.Pop_variation(gen, 0);
        return;
    }

    std::vector<double> targets(pops.size(), 0.0);
    std::vector<int> next_numbers(pops.size(), 0);
    std::vector<size_t> remainder_order(pops.size());
    double target_sum = 0;
    int floor_sum = 0;

    for (size_t index = 0; index < pops.size(); ++index) {
        Pop& pop = pops[index];
        remainder_order[index] = index;
        if (pop.number <= 0) continue;

        double group_capacity = std::max(0.0f, total_carrying_capacity)
            * pop.number / total_population;
        if (pop.reproduction_rate == 0) {
            targets[index] = pop.number;
        } else if (group_capacity > 0) {
            double denominator = 1.0
                + (group_capacity / pop.number - 1.0)
                    * std::exp(-pop.reproduction_rate);
            targets[index] = group_capacity / denominator;
        }

        next_numbers[index] = static_cast<int>(std::floor(targets[index]));
        floor_sum += next_numbers[index];
        target_sum += targets[index];
    }

    int persons_to_assign = static_cast<int>(std::lround(target_sum)) - floor_sum;
    std::stable_sort(remainder_order.begin(), remainder_order.end(),
        [&targets](size_t left, size_t right) {
            double left_remainder = targets[left] - std::floor(targets[left]);
            double right_remainder = targets[right] - std::floor(targets[right]);
            return left_remainder > right_remainder;
        });
    for (int index = 0; index < persons_to_assign; ++index) {
        ++next_numbers[remainder_order[index]];
    }

    for (size_t index = 0; index < pops.size(); ++index) {
        Pop& pop = pops[index];
        pop.number = next_numbers[index];
        if (pop.number <= 0) {
            pop.number = 0;
            pop.Goods["Food"] = 0;
            pop.Goods["Cloth"] = 0;
        } else if (pop.Goods["Cloth"] <= 0) {
            pop.Goods["Cloth"] = 0;
        }
    }
}

// Standard of Living: adjusts consumption priorities each iteration.
//   - Food_importance floats between 0.01 and 0.99 (self-correcting).
//   - When fed: Food_importance drifts down (prefer cloth/leisure).
//   - When starving: Food_importance rises, may trigger personal_loan.
//   - Unemployed pops bid from savings, not salary.
//   - Outputs: Money_for_Food and Money_for_Cloth (per-unit bid prices).
void Pop::SOL(std::mt19937& eng) {
    if (number <= 0) {
        personal_loan = 0;
        Consumption["Money_for_Food"] = 0;
        Consumption["Money_for_Cloth"] = 0;
        return;
    }

    double worker_salary = (factory != nullptr) ? factory->salary : 0.0;
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
            personal_loan = (Consumption["Food_importance"] - 0.99) * worker_salary;

        }
        if (Consumption["Food_importance"] > 0.99 && money < personal_loan) {
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
    float tax_rate = (gov != nullptr) ? gov->tax_rate : 0.0f;
    double net_salary = (factory != nullptr) ? factory->salary * (1 - tax_rate) : 0.0;
    double savings_income = money / 12.0;  // spend up to 1/12 of savings per iteration
    double bid_income = std::max(net_salary, savings_income);
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
    if (factory != nullptr && factory->workforce > 0) {
        float tax_rate = (gov != nullptr) ? gov->tax_rate : 0.0f;
        double wage_share = double(number) / factory->workforce;
        double salary_net = factory->salary * wage_share * (1 - tax_rate);
        if (gov != nullptr)
            gov->reserve += tax_rate * factory->salary * wage_share;
        money = money + salary_net;
    }
}

#include <iostream>
#include <math.h>
#include <map>
#include <fstream>
#include <iomanip>
#include <random>
#include <vector>
#include <chrono>
#include "lib/json.hpp"
#include <string>
#include "Agents/Factory.h"
#include "Agents/Market.h"
#include "Agents/Pop.h"
#include "Agents/Government.h"
using namespace nlohmann;
using namespace std;

int main() {
    std::ios::sync_with_stdio(false);
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    // Initialize json files
    std::ifstream production_methods("../Production_methods.json");
    std::ifstream Pop_("../Pop.json");
    std::ifstream Defines("../Defines.json");
    json Pop_j = json::parse(Pop_);
    json production_methods_j = json::parse(production_methods);
    json Defines_j = json::parse(Defines);

    int n_agents = Defines_j["n_agents"];
    int ITERATIONS = Defines_j["ITERATIONS"];
    int STAT = Defines_j["STAT"];
    float TAX_RATE = Defines_j["TAX_RATE"];
    int INITIAL_MONEY = Defines_j["INITIAL_MONEY"];
    unsigned RANDOM_SEED = Defines_j["RANDOM_SEED"];

    std::mt19937 gen(RANDOM_SEED);
    std::uniform_int_distribution<> factory_type(0, 1);
    std::uniform_int_distribution<> initial_population(1, STAT);

    Government govs;
    vector<Factory *> factories;
    vector<Pop> pops;
    vector<Pop*> unemployed;

    // Populate factories and pops
    for (int i = 0; i <= n_agents; i++) {
        factories.push_back(nullptr);
        int pop_num = initial_population(gen);
        pops.push_back(Pop(pop_num, INITIAL_MONEY, pop_num, nullptr, nullptr, &govs, Pop_j["Pop"]));
    }
    Market test_market(&pops, &factories);

    for (int i = 0; i <= n_agents; i++) {
        factories[i] = new Factory(&pops[i], &test_market, &govs,
                                   production_methods_j, factory_type(gen));
        pops[i].factory = factories[i];
        pops[i].markets = &test_market;
    }

    govs = Government(&factories, &pops, TAX_RATE, &test_market);

    // CSV setup
    ofstream myfile;
    myfile.open("data.csv");
    myfile << std::setprecision(10);
    myfile << "iteration," << "total population," << "average population,"
           << "food consumed," << "food produced," << "food carrying capacity,"
           << "food value," << "gdp"
           << ",unemployed"
           << ",cloth_consumed," << "cloth_produced," << "cloth_value"
           // Factory debug: food factories
           << ",Food_factories_count"
           << ",Food_avg_price,Food_min_price,Food_max_price"
           << ",Food_avg_markup,Food_avg_unit_cost"
           << ",Food_avg_salary,Food_avg_production,Food_avg_stock_sold"
           // Cloth factories
           << ",Cloth_factories_count"
           << ",Cloth_avg_price,Cloth_min_price,Cloth_max_price"
           << ",Cloth_avg_markup,Cloth_avg_unit_cost"
           << ",Cloth_avg_salary,Cloth_avg_production,Cloth_avg_stock_sold";
    // Per-pop summary: avg money, avg pop size, avg food importance
     myfile << ",Pop_avg_money,Pop_avg_number,Pop_avg_Food_importance,Pop_sample_size"
            << ",Factory_total_cash,Government_cash,Total_money_supply";
    for (int index = 0; index <= n_agents; index++) {
        myfile << ",Pop " << index << " food value"
               << ",Pop " << index << " Bought"
               << ",Pop " << index << " Population";
    }
     myfile << '\n';

     // Main loop
    for (int i = 0; i < ITERATIONS; i++) {
        auto t1 = high_resolution_clock::now();

        // Count salary recipients before factories run payroll.
        for (Factory*& entity : factories) {
            entity->worker_count = 0;
            entity->workforce = 0;
        }
        for (Pop& entity : pops) {
            if (entity.factory != nullptr && entity.number > 0) {
                entity.factory->worker_count++;
                entity.factory->workforce += entity.number;
            }
        }

        // 1. Factories produce and run payroll
        for (Factory*& entity : factories) {
            entity->Update();
        }

        // The paper's K(t)=I(t) linear case: one food unit supports one person
        // for one iteration. Snapshot population so agent update order cannot
        // change the proportional allocation of aggregate carrying capacity.
        double total_food_capacity = std::max(
            0.0f, test_market.Stats["Food_produced"]);

        // 2. Pops consume and receive payroll at their pre-growth size.
        for (Pop& entity : pops) {
            entity.Food_variation();
            entity.Money();
        }

        // Apply the demographic equation once at aggregate precision so the
        // arbitrary number of Pop groups cannot change the result by rounding.
        Pop::Population_variation(pops, gen, total_food_capacity);

        // 3. Pops set their bids at the updated population size.
        int n_cloth = 0;
        for (Pop& entity : pops) {
            entity.SOL(gen);
            n_cloth = n_cloth + entity.Goods["Cloth"] / std::max(1, entity.number);
        }

        cout << endl;

        // 4. Market clearing — posted-price, cheapest-first
        test_market.Update();

        // 5. Factories update their posted prices based on demand signal
        for (Factory*& entity : factories) {
            entity->CalculatePrice();
        }

        // 6. Labor mobility — pops may switch factories
        for (Pop& entity : pops) {
            entity.TrySwitchJob(factories, gen);
        }

        // 7. Track unemployed
        int unemployed_count = 0;
        for (Pop& entity : pops) {
            if (!entity.Employed()) unemployed_count++;
        }

        // 8. Factories adjust workforce (hire/fire)
        for (Factory*& entity : factories) {
            entity->AdjustWorkforce(pops, unemployed);
        }

        // 9. Update unemployed list
        unemployed.clear();
        for (Pop& entity : pops) {
            if (!entity.Employed()) {
                unemployed.push_back(&entity);
            }
        }

        // 10. Government census
        govs.Update();
        double average_population = pops.empty()
            ? 0.0
            : double(govs.n_tot) / pops.size();

        // Console dump
        cout << "    Number of iterations: " << i << std::endl;
        cout << "food price: " << test_market.Stats["Food_value"]
             << "     tot numbers: " << govs.n_tot
             << "     avg numbers: " << average_population
             << "     food needed " << test_market.Stats["Food_consumed"]
             << "     cloth per capita " << n_cloth / pops.size() << '\n';
        cout << "     gdp " << govs.gdp
             << "     reserve " << govs.reserve
             << "     food reserve " << govs.n_food
             << "     food produced " << test_market.Stats["Food_produced"]
             << "     carrying capacity " << total_food_capacity << '\n';
        cout << "     Cloth produced " << test_market.Stats["Cloth_produced"]
             << "     Cloth consumed " << test_market.Stats["Cloth_consumed"]
             << "     Cloth price: " << test_market.Stats["Cloth_value"]
             << "     Unemployed: " << unemployed_count;

        // File dump with factory debug
        myfile << i << ',' << govs.n_tot << ',' << average_population << ','
               << test_market.Stats["Food_consumed"] << ','
               << test_market.Stats["Food_produced"] << ','
               << total_food_capacity << ','
               << test_market.Stats["Food_value"] << ','
               << govs.gdp
               << ',' << unemployed_count
               << ',' << test_market.Stats["Cloth_consumed"]
               << ',' << test_market.Stats["Cloth_produced"]
               << ',' << test_market.Stats["Cloth_value"];

        // Factory summary per type (single pass)
        struct Stats { float sum, lo, hi, ms, uc, sa, pd, sd; int n; };
        Stats food{0, 1e30f, -1, 0, 0, 0, 0, 0, 0}, cloth{0, 1e30f, -1, 0, 0, 0, 0, 0, 0};
        for (Factory*& entity : factories) {
            Stats& s = (entity->type == "Food") ? food : cloth;
            s.n++;
            s.sum += entity->my_price;   s.ms += entity->my_markup;
            s.uc  += entity->unit_cost;  s.sa += entity->salary;
            s.pd  += entity->production; s.sd += entity->stock_sold;
            if (entity->my_price < s.lo) s.lo = entity->my_price;
            if (entity->my_price > s.hi) s.hi = entity->my_price;
        }
        for (Stats* s : {&food, &cloth}) {
            myfile << ',' << s->n
                   << ',' << (s->n ? s->sum/s->n : 0.f)
                   << ',' << (s->n ? s->lo : 0.f) << ',' << (s->n ? s->hi : 0.f)
                   << ',' << (s->n ? s->ms/s->n : 0.f)
                   << ',' << (s->n ? s->uc/s->n : 0.f)
                   << ',' << (s->n ? s->sa/s->n : 0.f)
                   << ',' << (s->n ? s->pd/s->n : 0.f)
                   << ',' << (s->n ? s->sd/s->n : 0.f);
        }

        // Per-pop summary
        double pop_sum_money = 0;
        double factory_sum_cash = 0;
        float pop_sum_number = 0, pop_sum_fi = 0;
        int pop_count = 0;
        for (Pop& entity : pops) {
            pop_sum_money += entity.money;
            pop_sum_number += entity.number;
            pop_sum_fi += entity.Consumption["Food_importance"];
            pop_count++;
        }
        for (Factory*& entity : factories) {
            factory_sum_cash += entity->cash;
        }
        myfile << ',' << (pop_sum_money / pop_count)
               << ',' << (pop_sum_number / pop_count)
               << ',' << (pop_sum_fi / pop_count)
               << ',' << pop_count
               << ',' << factory_sum_cash
               << ',' << govs.reserve
               << ',' << (pop_sum_money + factory_sum_cash + govs.reserve);
        for (int index = 0; index <= n_agents; index++) {
            myfile << ',' << pops[index].Consumption["Money_for_Food"]
                   << ',' << test_market.food_bought[index]
                   << ',' << pops[index].number;
        }
        myfile << '\n';

        // Reset accumulators
        govs.Reset();
        test_market.Stats["Food_consumed"] = 0;
        test_market.Stats["Food_produced"] = 0;
        test_market.Stats["Cloth_consumed"] = 0;
        test_market.Stats["Cloth_produced"] = 0;

        auto t2 = high_resolution_clock::now();
        cout << "    .Time: " << duration_cast<std::chrono::microseconds>(t2 - t1).count()
             << " microseconds" << std::endl;
    }

    myfile.close();
    for (Factory* factory : factories) delete factory;
    return 0;
}

#include <iostream>
#include <random>
#include <math.h>
#include <map>
#include <fstream>
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

int randoms(int y){
    static bool seeded = false;
    if (!seeded) {
        srand(time(nullptr));
        seeded = true;
    }
    int v= rand() % y + 1;
    return v;
};


int main() {
    std::ios::sync_with_stdio(false);
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, 1);
    std::uniform_real_distribution<float> roll(0.0f, 1.0f);

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

    Government govs;
    vector<Factory *> factories;
    vector<Pop> pops;
    vector<Pop*> unemployed;

    // Populate factories and pops
    for (int i = 0; i <= n_agents; i++) {
        factories.push_back(nullptr);
        pops.push_back(Pop(randoms(STAT), randoms(STAT), randoms(STAT), nullptr, nullptr, &govs, Pop_j["Pop"]));
    }
    Market test_market(&pops, &factories);

    for (int i = 0; i <= n_agents; i++) {
        factories[i] = new Factory(&pops[i], &test_market, &govs, production_methods_j, distr(gen));
        pops[i].factory = factories[i];
        pops[i].markets = &test_market;
    }

    govs = Government(&factories, &pops, TAX_RATE, &test_market);

    // CSV setup
    ofstream myfile;
    myfile.open("data.csv");
    myfile << "iteration," << "total population," << "average population,"
           << "food consumed," << "food produced," << "food value," << "gdp"
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
    myfile << endl;

    // Main loop
    for (int i = 0; i < ITERATIONS; i++) {
        auto t1 = high_resolution_clock::now();

        // 1. Factories produce and set prices
        for (Factory*& entity : factories) {
            entity->Update();
        }

        // 2. Pops: consume, earn, grow, set bids
        int n_cloth = 0;
        for (Pop& entity : pops) {
            entity.Food_variation();
            entity.Money();
            entity.Pop_variation();
            entity.SOL();
            n_cloth = n_cloth + entity.Goods["Cloth"] / std::max(1, entity.number);
        }

        cout << endl;

        // 3. Market clearing — posted-price, cheapest-first
        test_market.Update();

        // 4. Factories update their posted prices based on demand signal
        for (Factory*& entity : factories) {
            entity->CalculatePrice();
        }

        // 5. Labor mobility — pops may switch factories
        for (Pop& entity : pops) {
            entity.TrySwitchJob(factories, gen);
        }

        // 6. Track unemployed
        int unemployed_count = 0;
        for (Pop& entity : pops) {
            if (!entity.Employed()) unemployed_count++;
        }

        // 7. Factories adjust workforce (hire/fire)
        for (Factory*& entity : factories) {
            entity->AdjustWorkforce(pops, unemployed);
        }

        // 8. Update unemployed list
        unemployed.clear();
        for (Pop& entity : pops) {
            if (!entity.Employed()) {
                unemployed.push_back(&entity);
            }
        }

        // 9. Government census
        govs.Update();

        // Console dump
        cout << "    Number of iterations: " << i << std::endl;
        cout << "food price: " << test_market.Stats["Food_value"]
             << "     tot numbers: " << govs.n_tot
             << "     avg numbers: " << govs.n_tot / n_agents
             << "     food needed " << test_market.Stats["Food_consumed"]
             << "     cloth per capita " << n_cloth / n_agents << '\n';
        cout << "     gdp " << govs.gdp
             << "     reserve " << govs.reserve
             << "     food reserve " << govs.n_food
             << "     food produced " << test_market.Stats["Food_produced"] << '\n';
        cout << "     Cloth produced " << test_market.Stats["Cloth_produced"]
             << "     Cloth consumed " << test_market.Stats["Cloth_consumed"]
             << "     Cloth price: " << test_market.Stats["Cloth_value"]
             << "     Unemployed: " << unemployed_count;

        // File dump with factory debug
        myfile << i << ',' << govs.n_tot << ',' << govs.n_tot / n_agents << ','
               << test_market.Stats["Food_consumed"] << ','
               << test_market.Stats["Food_produced"] << ','
               << test_market.Stats["Food_value"] << ','
               << govs.gdp
               << ',' << unemployed_count
               << ',' << test_market.Stats["Cloth_consumed"]
               << ',' << test_market.Stats["Cloth_produced"]
               << ',' << test_market.Stats["Cloth_value"];

        // Factory summary per type
        for (auto good_type : {"Food", "Cloth"}) {
            float sum_price = 0, min_price = 1e30f, max_price = -1;
            float sum_markup = 0, sum_unit_cost = 0;
            float sum_salary = 0, sum_prod = 0, sum_sold = 0;
            int count = 0;
            for (Factory*& entity : factories) {
                if (entity->type != good_type) continue;
                count++;
                sum_price += entity->my_price;
                sum_markup += entity->my_markup;
                sum_unit_cost += entity->unit_cost;
                sum_salary += entity->salary;
                sum_prod += entity->production;
                sum_sold += entity->stock_sold;
                if (entity->my_price < min_price) min_price = entity->my_price;
                if (entity->my_price > max_price) max_price = entity->my_price;
            }
            float avg_price = count ? sum_price / count : 0;
            float avg_markup = count ? sum_markup / count : 0;
            float avg_unit_cost = count ? sum_unit_cost / count : 0;
            float avg_salary = count ? sum_salary / count : 0;
            float avg_prod = count ? sum_prod / count : 0;
            float avg_sold = count ? sum_sold / count : 0;
            myfile << ',' << count
                   << ',' << avg_price << ',' << (count ? min_price : 0) << ',' << (count ? max_price : 0)
                   << ',' << avg_markup << ',' << avg_unit_cost
                   << ',' << avg_salary << ',' << avg_prod << ',' << avg_sold;
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
    getchar();
    return 0;
}

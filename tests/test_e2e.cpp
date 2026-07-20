#include "test_utils.h"
#include "../Agents/Pop.h"
#include "../Agents/Factory.h"
#include "../Agents/Market.h"
#include "../Agents/Government.h"
#include "../lib/json.hpp"
#include <fstream>
#include <cmath>
#include <vector>
#include <random>
#include <string>
#include <map>
#include <algorithm>
#include <sstream>

using namespace nlohmann;
using namespace std;

int randoms(int y) {
    static bool seeded = false;
    if (!seeded) {
        srand(time(nullptr));
        seeded = true;
    }
    return rand() % y + 1;
}

struct SimulationResult {
    float final_food_price = 0;
    float final_cloth_price = 0;
    float final_gov_reserve = 0;
    int final_total_population = 0;
    float final_gdp = 0;
    float final_food_produced = 0;
    float final_cloth_produced = 0;
    float final_food_consumed = 0;
    float final_cloth_consumed = 0;
    float final_avg_population = 0;
    float final_avg_pop_money = 0;
    int iterations_run = 0;

    vector<float> food_price_hist;
    vector<float> cloth_price_hist;
    vector<float> gov_reserve_hist;
    vector<int> total_pop_hist;
    vector<float> gdp_hist;
    vector<float> food_produced_hist;
    vector<float> cloth_produced_hist;
    vector<float> food_consumed_hist;
    vector<float> cloth_consumed_hist;
};

json make_production_methods_json() {
    return {
        {"Food", {
            {"Production_methods", {
                {"PM_1", {
                    {"Efficiency_parameter", 20},
                    {"Sigma", 0.8},
                    {"Investment_relation", 0.1},
                    {"distribution_parameter", 0.35}
                }}
            }}
        }},
        {"Cloth", {
            {"Production_methods", {
                {"PM_1", {
                    {"Efficiency_parameter", 2},
                    {"Sigma", 0.8},
                    {"Investment_relation", 0.1},
                    {"distribution_parameter", 0.35}
                }}
            }}
        }}
    };
}

json make_pop_json() {
    return {
        {"Pop", {
            {"reserve_constant", 1},
            {"test", 0.1},
            {"reproduction_rate", 0.05}
        }}
    };
}

SimulationResult run_simulation(int n_agents, int iterations, int stat, float tax_rate) {
    SimulationResult result;
    result.iterations_run = iterations;

    json production_methods_j = make_production_methods_json();
    json Pop_j = make_pop_json();

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distr(0, 1);

    Government govs;
    vector<Factory*> factories;
    vector<Pop> pops;
    Market test_market(&pops);

    for (int i = 0; i <= n_agents; i++) {
        factories.push_back(nullptr);
        pops.push_back(Pop(randoms(stat), randoms(stat), randoms(stat), nullptr,
                           &test_market, &govs, Pop_j["Pop"]));
    }
    for (int i = 0; i <= n_agents; i++) {
        factories[i] = new Factory(&pops[i], &test_market, &govs,
                                   production_methods_j, distr(gen));
        pops[i].factory = factories[i];
    }

    govs = Government(&factories, &pops, tax_rate, &test_market);

    for (int i = 0; i < iterations; i++) {
        for (Factory*& entity : factories) {
            entity->Update();
        }
        for (Pop& entity : pops) {
            entity.Food_variation();
            entity.Money();
            entity.Pop_variation();
            entity.SOL();
        }
        test_market.Update();
        govs.Update();

        result.food_price_hist.push_back(test_market.Stats["Food_value"]);
        result.cloth_price_hist.push_back(test_market.Stats["Cloth_value"]);
        result.gov_reserve_hist.push_back(govs.reserve);
        result.total_pop_hist.push_back(govs.n_tot);
        result.gdp_hist.push_back(govs.gdp);
        result.food_produced_hist.push_back(test_market.Stats["Food_produced"]);
        result.cloth_produced_hist.push_back(test_market.Stats["Cloth_produced"]);
        result.food_consumed_hist.push_back(test_market.Stats["Food_consumed"]);
        result.cloth_consumed_hist.push_back(test_market.Stats["Cloth_consumed"]);

        govs.Reset();
        test_market.Stats["Food_consumed"] = 0;
        test_market.Stats["Food_produced"] = 0;
        test_market.Stats["Cloth_consumed"] = 0;
        test_market.Stats["Cloth_produced"] = 0;
    }

    result.final_food_price = test_market.Stats["Food_value"];
    result.final_cloth_price = test_market.Stats["Cloth_value"];
    result.final_gov_reserve = govs.reserve;
    result.final_gdp = govs.gdp;
    result.final_food_produced = test_market.Stats["Food_produced"];
    result.final_cloth_produced = test_market.Stats["Cloth_produced"];
    result.final_food_consumed = test_market.Stats["Food_consumed"];
    result.final_cloth_consumed = test_market.Stats["Cloth_consumed"];

    float total_money = 0;
    int total_pop = 0;
    for (auto& p : pops) {
        total_pop += p.number;
        total_money += p.money;
    }
    result.final_total_population = total_pop;
    result.final_avg_population = static_cast<float>(total_pop) / (n_agents + 1);
    result.final_avg_pop_money = total_money / (n_agents + 1);

    for (auto* f : factories) {
        delete f;
    }

    return result;
}

SimulationResult run_simulation_with_types(int n_agents, int iterations, int stat,
                                            float tax_rate, const vector<int>& factory_types) {
    SimulationResult result;
    result.iterations_run = iterations;

    json production_methods_j = make_production_methods_json();
    json Pop_j = make_pop_json();

    Government govs;
    vector<Factory*> factories;
    vector<Pop> pops;
    Market test_market(&pops);

    int num_entities = n_agents + 1;
    if (static_cast<int>(factory_types.size()) != num_entities) {
        num_entities = static_cast<int>(factory_types.size());
    }

    for (int i = 0; i < num_entities; i++) {
        factories.push_back(nullptr);
        pops.push_back(Pop(randoms(stat), randoms(stat), randoms(stat), nullptr,
                           &test_market, &govs, Pop_j["Pop"]));
    }
    for (int i = 0; i < num_entities; i++) {
        int type_int = factory_types[i];
        factories[i] = new Factory(&pops[i], &test_market, &govs,
                                   production_methods_j, type_int);
        pops[i].factory = factories[i];
    }

    govs = Government(&factories, &pops, tax_rate, &test_market);

    for (int i = 0; i < iterations; i++) {
        for (Factory*& entity : factories) {
            entity->Update();
        }
        for (Pop& entity : pops) {
            entity.Food_variation();
            entity.Money();
            entity.Pop_variation();
            entity.SOL();
        }
        test_market.Update();
        govs.Update();

        result.food_price_hist.push_back(test_market.Stats["Food_value"]);
        result.cloth_price_hist.push_back(test_market.Stats["Cloth_value"]);
        result.gov_reserve_hist.push_back(govs.reserve);
        result.total_pop_hist.push_back(govs.n_tot);
        result.gdp_hist.push_back(govs.gdp);
        result.food_produced_hist.push_back(test_market.Stats["Food_produced"]);
        result.cloth_produced_hist.push_back(test_market.Stats["Cloth_produced"]);
        result.food_consumed_hist.push_back(test_market.Stats["Food_consumed"]);
        result.cloth_consumed_hist.push_back(test_market.Stats["Cloth_consumed"]);

        govs.Reset();
        test_market.Stats["Food_consumed"] = 0;
        test_market.Stats["Food_produced"] = 0;
        test_market.Stats["Cloth_consumed"] = 0;
        test_market.Stats["Cloth_produced"] = 0;
    }

    result.final_food_price = test_market.Stats["Food_value"];
    result.final_cloth_price = test_market.Stats["Cloth_value"];
    result.final_gov_reserve = govs.reserve;
    result.final_gdp = govs.gdp;
    result.final_food_produced = test_market.Stats["Food_produced"];
    result.final_cloth_produced = test_market.Stats["Cloth_produced"];
    result.final_food_consumed = test_market.Stats["Food_consumed"];
    result.final_cloth_consumed = test_market.Stats["Cloth_consumed"];

    float total_money = 0;
    int total_pop = 0;
    for (auto& p : pops) {
        total_pop += p.number;
        total_money += p.money;
    }
    result.final_total_population = total_pop;
    result.final_avg_population = static_cast<float>(total_pop) / num_entities;
    result.final_avg_pop_money = total_money / num_entities;

    for (auto* f : factories) {
        delete f;
    }

    return result;
}

TEST(single_agent_smoke_test) {
    auto result = run_simulation(1, 5, 100, 0.0f);

    for (float price : result.food_price_hist) {
        EXPECT_FALSE(std::isnan(price));
        EXPECT_FALSE(std::isinf(price));
        EXPECT_GE(price, 0.0f);
    }
    for (float price : result.cloth_price_hist) {
        EXPECT_FALSE(std::isnan(price));
        EXPECT_FALSE(std::isinf(price));
        EXPECT_GE(price, 0.0f);
    }
    for (int pop : result.total_pop_hist) {
        EXPECT_GT(pop, 0);
    }
    for (float gdp : result.gdp_hist) {
        EXPECT_FALSE(std::isnan(gdp));
        EXPECT_FALSE(std::isinf(gdp));
    }
    for (float reserve : result.gov_reserve_hist) {
        EXPECT_FALSE(std::isnan(reserve));
        EXPECT_FALSE(std::isinf(reserve));
    }

    EXPECT_GT(result.final_food_price, 0.0f);
    EXPECT_GE(result.final_cloth_price, 0.0f);
    EXPECT_GT(result.final_total_population, 0);
    EXPECT_FALSE(std::isnan(result.final_avg_pop_money));
    EXPECT_FALSE(std::isinf(result.final_avg_pop_money));
}

TEST(small_simulation_flow) {
    auto result = run_simulation(3, 20, 100, 0.0f);

    bool food_produced = false;
    bool cloth_produced = false;
    for (int i = 0; i < result.iterations_run; i++) {
        if (result.food_produced_hist[i] > 0) food_produced = true;
        if (result.cloth_produced_hist[i] > 0) cloth_produced = true;
    }
    EXPECT_TRUE(food_produced || cloth_produced);

    bool food_consumed = false;
    bool cloth_consumed = false;
    for (int i = 0; i < result.iterations_run; i++) {
        if (result.food_consumed_hist[i] > 0) food_consumed = true;
        if (result.cloth_consumed_hist[i] > 0) cloth_consumed = true;
    }
    EXPECT_TRUE(food_consumed || cloth_consumed);

    bool gdp_positive = false;
    for (int i = 0; i < result.iterations_run; i++) {
        EXPECT_FALSE(std::isinf(result.gdp_hist[i]));
        EXPECT_FALSE(std::isnan(result.gdp_hist[i]));
        if (result.gdp_hist[i] > 0) gdp_positive = true;
    }
    EXPECT_TRUE(gdp_positive);

    EXPECT_GE(result.final_food_price, 0.0f);
    EXPECT_LT(result.final_food_price, 1000000.0f);
    EXPECT_GE(result.final_cloth_price, 0.0f);
    EXPECT_LT(result.final_cloth_price, 1000000.0f);

    EXPECT_FALSE(std::isinf(result.final_avg_pop_money));
    EXPECT_FALSE(std::isnan(result.final_avg_pop_money));
    EXPECT_LT(result.final_avg_pop_money, 1e12f);

    EXPECT_GT(result.final_total_population, 0);

    for (int i = 0; i < result.iterations_run; i++) {
        float expected_gdp =
            result.food_produced_hist[i] * result.food_price_hist[i] +
            result.cloth_produced_hist[i] * result.cloth_price_hist[i];
        EXPECT_FLOAT_EQ(result.gdp_hist[i], expected_gdp, 0.1f);
    }
}

TEST(mixed_food_cloth_factories) {
    vector<int> types = {0, 0, 1, 1};
    auto result = run_simulation_with_types(3, 10, 100, 0.0f, types);

    bool food_produced = false;
    bool cloth_produced = false;
    for (int i = 0; i < result.iterations_run; i++) {
        if (result.food_produced_hist[i] > 0) food_produced = true;
        if (result.cloth_produced_hist[i] > 0) cloth_produced = true;
    }
    EXPECT_TRUE(food_produced);
    EXPECT_TRUE(cloth_produced);

    bool food_consumed = false;
    bool cloth_consumed_positive = false;
    for (int i = 0; i < result.iterations_run; i++) {
        if (result.food_consumed_hist[i] > 0) food_consumed = true;
        if (result.cloth_consumed_hist[i] > 0) cloth_consumed_positive = true;
    }
    EXPECT_TRUE(food_consumed);
    EXPECT_TRUE(cloth_consumed_positive);

    for (float price : result.food_price_hist) {
        EXPECT_FALSE(std::isnan(price));
        EXPECT_FALSE(std::isinf(price));
    }
    for (float price : result.cloth_price_hist) {
        EXPECT_FALSE(std::isnan(price));
        EXPECT_FALSE(std::isinf(price));
    }
}

TEST(population_dynamics) {
    auto result_high = run_simulation(3, 30, 1000, 0.0f);
    auto result_low = run_simulation(3, 30, 1, 0.0f);

    float start_pop_high = result_high.total_pop_hist[0];
    float end_pop_high = result_high.total_pop_hist.back();

    for (int pop : result_high.total_pop_hist) {
        EXPECT_GT(pop, 0);
    }
    for (int pop : result_low.total_pop_hist) {
        EXPECT_GT(pop, 0);
        EXPECT_GE(pop, 1);
    }

    bool any_change = false;
    for (int i = 1; i < result_high.iterations_run; i++) {
        if (result_high.total_pop_hist[i] != result_high.total_pop_hist[i - 1]) {
            any_change = true;
            break;
        }
    }
    EXPECT_TRUE(any_change);

    bool any_change_low = false;
    for (int i = 1; i < result_low.iterations_run; i++) {
        if (result_low.total_pop_hist[i] != result_low.total_pop_hist[i - 1]) {
            any_change_low = true;
            break;
        }
    }
}

TEST(market_price_discovery) {
    auto result = run_simulation(3, 30, 100, 0.0f);

    EXPECT_GT(result.food_price_hist[0], 0.0f);
    EXPECT_LT(result.food_price_hist[0], 1000000.0f);
    EXPECT_GT(result.cloth_price_hist[0], 0.0f);
    EXPECT_LT(result.cloth_price_hist[0], 1000000.0f);

    bool food_price_changed = false;
    for (int i = 1; i < result.iterations_run; i++) {
        if (std::fabs(result.food_price_hist[i] - 100.0f) > 0.01f) {
            food_price_changed = true;
            break;
        }
    }
    EXPECT_TRUE(food_price_changed);

    bool cloth_price_changed = false;
    for (int i = 1; i < result.iterations_run; i++) {
        if (std::fabs(result.cloth_price_hist[i] - 100.0f) > 0.01f) {
            cloth_price_changed = true;
            break;
        }
    }
    EXPECT_TRUE(cloth_price_changed);

    for (int i = 0; i < result.iterations_run; i++) {
        EXPECT_FALSE(std::isnan(result.food_price_hist[i]));
        EXPECT_FALSE(std::isinf(result.food_price_hist[i]));
        EXPECT_FALSE(std::isnan(result.cloth_price_hist[i]));
        EXPECT_FALSE(std::isinf(result.cloth_price_hist[i]));
        EXPECT_GE(result.food_price_hist[i], 0.0f);
        EXPECT_GE(result.cloth_price_hist[i], 0.0f);
    }
}

TEST(taxation) {
    auto result = run_simulation(3, 20, 100, 0.1f);

    EXPECT_GT(result.final_gov_reserve, 0.0f);

    for (int i = 1; i < result.iterations_run; i++) {
        EXPECT_GE(result.gov_reserve_hist[i], result.gov_reserve_hist[i - 1]);
    }

    auto result_no_tax = run_simulation(3, 20, 100, 0.0f);
    EXPECT_FLOAT_EQ(result_no_tax.final_gov_reserve, 0.0f, 0.01f);
}

TEST(csv_output_validation) {
    json production_methods_j = make_production_methods_json();
    json Pop_j = make_pop_json();

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distr(0, 1);

    int n_agents = 1;
    int iterations = 5;
    int stat = 100;

    Government govs;
    vector<Factory*> factories;
    vector<Pop> pops;
    Market test_market(&pops);

    for (int i = 0; i <= n_agents; i++) {
        factories.push_back(nullptr);
        pops.push_back(Pop(randoms(stat), randoms(stat), randoms(stat), nullptr,
                           &test_market, &govs, Pop_j["Pop"]));
    }
    for (int i = 0; i <= n_agents; i++) {
        factories[i] = new Factory(&pops[i], &test_market, &govs,
                                   production_methods_j, distr(gen));
        pops[i].factory = factories[i];
    }

    govs = Government(&factories, &pops, 0.0f, &test_market);

    ofstream myfile;
    myfile.open("data.csv");

    myfile << "iteration," << "total population," << "average population,"
           << "food consumed," << "food produced," << "food value,"
           << "gdp";
    for (int i = 0; i <= n_agents; i++) {
        myfile << ",Pop " << i << " food value"
               << ",Pop index " << i
               << ",Pop " << i << " Bought"
               << ",Pop " << i << " Population"
               << ",Pop " << i << " Money"
               << ",Pop " << i << " Food Importance";
    }
    myfile << endl;

    for (int i = 0; i < iterations; i++) {
        for (Factory*& entity : factories) {
            entity->Update();
        }
        for (Pop& entity : pops) {
            entity.Food_variation();
            entity.Money();
            entity.Pop_variation();
            entity.SOL();
        }
        test_market.Update();
        govs.Update();

        myfile << i << ',' << govs.n_tot << ',' << govs.n_tot / n_agents << ','
               << test_market.Stats["Food_consumed"] << ','
               << test_market.Stats["Food_produced"] << ','
               << test_market.Stats["Food_value"] << ','
               << govs.gdp;

        for (int x = 0; x < static_cast<int>(test_market.food_market.size()); x++) {
            myfile << ',' << test_market.food_market[x][1]
                   << ',' << test_market.food_market[x][0]
                   << ',' << test_market.food_market[x][2]
                   << ',' << pops[static_cast<int>(test_market.food_market[x][0])].number
                   << ',' << pops[static_cast<int>(test_market.food_market[x][0])].money
                   << ',' << pops[static_cast<int>(test_market.food_market[x][0])].Consumption["Food_importance"];
        }
        myfile << '\n';

        govs.Reset();
        test_market.Stats["Food_consumed"] = 0;
        test_market.Stats["Food_produced"] = 0;
        test_market.Stats["Cloth_consumed"] = 0;
        test_market.Stats["Cloth_produced"] = 0;
    }

    myfile.close();

    ifstream infile("data.csv");
    EXPECT_TRUE(infile.good());

    string header_line;
    EXPECT_TRUE(std::getline(infile, header_line));
    EXPECT_TRUE(header_line.find("iteration") != string::npos);
    EXPECT_TRUE(header_line.find("total population") != string::npos);
    EXPECT_TRUE(header_line.find("average population") != string::npos);
    EXPECT_TRUE(header_line.find("food consumed") != string::npos);
    EXPECT_TRUE(header_line.find("food produced") != string::npos);
    EXPECT_TRUE(header_line.find("food value") != string::npos);
    EXPECT_TRUE(header_line.find("gdp") != string::npos);
    EXPECT_TRUE(header_line.find("Pop 0 food value") != string::npos);
    EXPECT_TRUE(header_line.find("Population") != string::npos);
    EXPECT_TRUE(header_line.find("Money") != string::npos);
    EXPECT_TRUE(header_line.find("Food Importance") != string::npos);

    int row_count = 0;
    string data_line;
    while (std::getline(infile, data_line)) {
        if (data_line.empty()) continue;
        EXPECT_TRUE(data_line.find(",,") == string::npos);

        istringstream ss(data_line);
        string cell;
        int col_count = 0;
        while (std::getline(ss, cell, ',')) {
            EXPECT_TRUE(!cell.empty());
            col_count++;
        }
        EXPECT_GT(col_count, 0);

        istringstream ss2(data_line);
        string iter_str;
        std::getline(ss2, iter_str, ',');
        int iter_val = stoi(iter_str);
        EXPECT_EQ(iter_val, row_count);

        row_count++;
    }
    EXPECT_EQ(row_count, iterations);

    infile.close();
    remove("data.csv");

    for (auto* f : factories) {
        delete f;
    }
}

TEST(edge_case_zero_agents) {
    auto result = run_simulation(0, 5, 100, 0.0f);

    EXPECT_EQ(result.food_price_hist.size(), static_cast<size_t>(5));
    EXPECT_GT(result.final_total_population, 0);

    for (float price : result.food_price_hist) {
        EXPECT_FALSE(std::isnan(price));
        EXPECT_FALSE(std::isinf(price));
        EXPECT_GE(price, 0.0f);
    }
    for (float price : result.cloth_price_hist) {
        EXPECT_FALSE(std::isnan(price));
        EXPECT_FALSE(std::isinf(price));
        EXPECT_GE(price, 0.0f);
    }
}

TEST(edge_case_all_same_good) {
    vector<int> types = {0, 0, 0, 0};
    auto result = run_simulation_with_types(3, 10, 100, 0.0f, types);

    bool food_produced = false;
    for (int i = 0; i < result.iterations_run; i++) {
        if (result.food_produced_hist[i] > 0) food_produced = true;
        EXPECT_FLOAT_EQ(result.cloth_produced_hist[i], 0.0f, 0.01f);
    }
    EXPECT_TRUE(food_produced);

    for (float price : result.food_price_hist) {
        EXPECT_FALSE(std::isnan(price));
        EXPECT_FALSE(std::isinf(price));
    }
    for (int pop : result.total_pop_hist) {
        EXPECT_GT(pop, 0);
    }
}

TEST(edge_case_extreme_stat) {
    auto result_low = run_simulation(2, 10, 1, 0.0f);
    EXPECT_GT(result_low.final_total_population, 0);
    for (float price : result_low.food_price_hist) {
        EXPECT_FALSE(std::isnan(price));
        EXPECT_FALSE(std::isinf(price));
    }
    for (int pop : result_low.total_pop_hist) {
        EXPECT_GT(pop, 0);
    }

    auto result_high = run_simulation(2, 10, 10000, 0.0f);
    EXPECT_GT(result_high.final_total_population, 0);
    for (float price : result_high.food_price_hist) {
        EXPECT_FALSE(std::isnan(price));
        EXPECT_FALSE(std::isinf(price));
    }
    for (int pop : result_high.total_pop_hist) {
        EXPECT_GT(pop, 0);
    }
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}

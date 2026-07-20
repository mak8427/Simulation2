#include "test_utils.h"
#include "../Agents/Factory.h"
#include "../Agents/Government.h"
#include "../Agents/Market.h"
#include "../Agents/Pop.h"

#include <cmath>
#include <random>
#include <string>
#include <vector>

using namespace std;

static nlohmann::json production_config() {
    return {
        {"Food", {{"Production_methods", {{"PM_1", {
            {"Efficiency_parameter", 20}, {"Sigma", 0.8},
            {"distribution_parameter", 0.35}}}}}}},
        {"Cloth", {{"Production_methods", {{"PM_1", {
            {"Efficiency_parameter", 2}, {"Sigma", 0.8},
            {"distribution_parameter", 0.35}}}}}}}
    };
}

static nlohmann::json pop_config() {
    return {{"reserve_constant", 1.0f}, {"reproduction_rate", 0.05f}};
}

struct SimulationResult {
    vector<int> population;
    vector<float> food_produced;
    vector<float> cloth_produced;
    vector<float> food_consumed;
    vector<float> cloth_consumed;
    vector<float> food_price;
    vector<float> cloth_price;
    vector<double> government_reserve;
    vector<double> total_money;
    vector<double> gdp;
    vector<double> total_salary;
    vector<int> active_workforce;
    bool all_balances_nonnegative = true;
};

static double economy_money(const vector<Pop>& pops,
                            const vector<Factory*>& factories,
                            const Government& government) {
    double total = government.reserve;
    for (const Pop& pop : pops) total += pop.money;
    for (const Factory* factory : factories) total += factory->cash;
    return total;
}

static SimulationResult run_simulation(const vector<int>& factory_types,
                                       int periods, float tax_rate,
                                       int initial_population = 100,
                                       double initial_money = 5000,
                                       unsigned seed = 12345) {
    SimulationResult result;
    vector<Pop> pops;
    vector<Factory*> factories(factory_types.size(), nullptr);
    pops.reserve(factory_types.size());

    Government government;
    Market market(&pops, &factories);
    government = Government(&factories, &pops, tax_rate, &market);
    mt19937 gen(seed);

    for (size_t index = 0; index < factory_types.size(); ++index) {
        pops.emplace_back(initial_population, static_cast<int>(initial_money),
                          initial_population, nullptr, &market, &government,
                          pop_config());
    }
    for (size_t index = 0; index < factory_types.size(); ++index) {
        factories[index] = new Factory(&pops[index], &market, &government,
                                       production_config(), factory_types[index]);
        factories[index]->my_price = 1.0f;
        pops[index].factory = factories[index];
    }

    const double initial_total_money = economy_money(pops, factories, government);

    for (int period = 0; period < periods; ++period) {
        int workforce = 0;
        for (Factory* factory : factories) {
            factory->worker_count = 0;
            factory->workforce = 0;
        }
        for (Pop& pop : pops) {
            if (pop.factory != nullptr && pop.number > 0) {
                pop.factory->worker_count++;
                pop.factory->workforce += pop.number;
                workforce += pop.number;
            }
        }
        result.active_workforce.push_back(workforce);

        for (Factory* factory : factories) factory->Update();
        for (Pop& pop : pops) {
            pop.Food_variation();
            pop.Money();
        }
        Pop::Population_variation(pops, gen, market.Stats["Food_produced"]);
        for (Pop& pop : pops) {
            pop.SOL(gen);
        }

        market.Update();
        for (Factory* factory : factories) factory->CalculatePrice();

        government.Reset();
        government.Update();

        double salary = 0;
        for (Factory* factory : factories) salary += factory->salary;
        result.population.push_back(government.n_tot);
        result.food_produced.push_back(market.Stats["Food_produced"]);
        result.cloth_produced.push_back(market.Stats["Cloth_produced"]);
        result.food_consumed.push_back(market.Stats["Food_consumed"]);
        result.cloth_consumed.push_back(market.Stats["Cloth_consumed"]);
        result.food_price.push_back(market.Stats["Food_value"]);
        result.cloth_price.push_back(market.Stats["Cloth_value"]);
        result.government_reserve.push_back(government.reserve);
        result.gdp.push_back(government.gdp);
        result.total_salary.push_back(salary);
        result.total_money.push_back(economy_money(pops, factories, government));

        for (const Pop& pop : pops) {
            if (pop.money < -0.0001) result.all_balances_nonnegative = false;
        }
        for (const Factory* factory : factories) {
            if (factory->cash < -0.0001) result.all_balances_nonnegative = false;
        }
        if (government.reserve < -0.0001) result.all_balances_nonnegative = false;
        if (std::fabs(result.total_money.back() - initial_total_money) > 0.001)
            result.all_balances_nonnegative = false;

        market.Stats["Food_produced"] = 0;
        market.Stats["Food_consumed"] = 0;
        market.Stats["Cloth_produced"] = 0;
        market.Stats["Cloth_consumed"] = 0;
    }

    for (Factory* factory : factories) delete factory;
    return result;
}

static bool any_positive(const vector<float>& values) {
    for (float value : values) {
        if (value > 0) return true;
    }
    return false;
}

TEST(mixed_economy_runs_main_phase_order_with_finite_state) {
    SimulationResult result = run_simulation({0, 0, 1, 1}, 8, 0.1f);

    EXPECT_EQ(result.population.size(), static_cast<size_t>(8));
    EXPECT_TRUE(any_positive(result.food_produced));
    EXPECT_TRUE(any_positive(result.cloth_produced));
    EXPECT_TRUE(any_positive(result.food_consumed));
    EXPECT_TRUE(result.all_balances_nonnegative);

    for (size_t period = 0; period < result.population.size(); ++period) {
        EXPECT_GE(result.population[period], 0);
        EXPECT_TRUE(std::isfinite(result.food_price[period]));
        EXPECT_TRUE(std::isfinite(result.cloth_price[period]));
        EXPECT_TRUE(std::isfinite(result.gdp[period]));
        EXPECT_TRUE(std::isfinite(result.total_money[period]));
    }
}

TEST(all_periods_conserve_initial_money_across_pops_factories_and_government) {
    SimulationResult result = run_simulation({0, 0, 1, 1}, 12, 0.2f);
    EXPECT_TRUE(result.all_balances_nonnegative);
    for (double total : result.total_money) {
        EXPECT_FLOAT_EQ(total, result.total_money.front(), 0.001);
    }
}

TEST(taxation_moves_money_to_reserve_without_changing_total_money) {
    SimulationResult taxed = run_simulation({0, 0}, 6, 0.25f);
    SimulationResult untaxed = run_simulation({0, 0}, 6, 0.0f);

    EXPECT_GT(taxed.government_reserve.back(), 0.0);
    EXPECT_FLOAT_EQ(untaxed.government_reserve.back(), 0.0, 0.0001);
    for (size_t period = 1; period < taxed.government_reserve.size(); ++period) {
        EXPECT_GE(taxed.government_reserve[period],
                  taxed.government_reserve[period - 1]);
    }
    EXPECT_FLOAT_EQ(taxed.total_money.back(), untaxed.total_money.back(), 0.001);
}

TEST(government_gdp_matches_current_output_at_current_market_prices) {
    SimulationResult result = run_simulation({0, 1}, 6, 0.0f);

    for (size_t period = 0; period < result.gdp.size(); ++period) {
        double expected = result.food_produced[period] * result.food_price[period]
                        + result.cloth_produced[period] * result.cloth_price[period];
        EXPECT_FLOAT_EQ(result.gdp[period], expected, 0.01);
    }
}

TEST(workforce_is_recalculated_from_current_population_each_period) {
    SimulationResult result = run_simulation({0}, 5, 0.0f, 100);

    EXPECT_EQ(result.active_workforce[0], 100);
    for (size_t period = 1; period < result.active_workforce.size(); ++period) {
        EXPECT_EQ(result.active_workforce[period], result.population[period - 1]);
    }
}

TEST(food_only_economy_never_produces_or_sells_cloth) {
    SimulationResult result = run_simulation({0, 0, 0}, 6, 0.0f);

    EXPECT_TRUE(any_positive(result.food_produced));
    for (size_t period = 0; period < result.cloth_produced.size(); ++period) {
        EXPECT_FLOAT_EQ(result.cloth_produced[period], 0.0f, 0.0001f);
        EXPECT_FLOAT_EQ(result.cloth_consumed[period], 0.0f, 0.0001f);
    }
}

TEST(no_food_production_allows_extinction_and_then_stops_all_output) {
    SimulationResult result = run_simulation({1, 1}, 5, 0.0f);

    EXPECT_EQ(result.population.back(), 0);
    EXPECT_EQ(result.active_workforce.back(), 0);
    EXPECT_FLOAT_EQ(result.cloth_produced.back(), 0.0f, 0.0001f);
    EXPECT_TRUE(result.all_balances_nonnegative);
}

TEST(empty_economy_remains_empty_and_conserves_zero_money) {
    SimulationResult result = run_simulation({}, 4, 0.2f);

    EXPECT_EQ(result.population.size(), static_cast<size_t>(4));
    for (size_t period = 0; period < result.population.size(); ++period) {
        EXPECT_EQ(result.population[period], 0);
        EXPECT_FLOAT_EQ(result.total_money[period], 0.0, 0.0001);
        EXPECT_FLOAT_EQ(result.gdp[period], 0.0, 0.0001);
    }
}

TEST(seeded_simulation_is_reproducible) {
    SimulationResult first = run_simulation({0, 1, 0}, 8, 0.1f, 100, 5000, 77);
    SimulationResult second = run_simulation({0, 1, 0}, 8, 0.1f, 100, 5000, 77);

    EXPECT_EQ(first.population.size(), second.population.size());
    for (size_t period = 0; period < first.population.size(); ++period) {
        EXPECT_EQ(first.population[period], second.population[period]);
        EXPECT_FLOAT_EQ(first.food_price[period], second.food_price[period],
                        0.0001f);
        EXPECT_FLOAT_EQ(first.cloth_price[period], second.cloth_price[period],
                        0.0001f);
        EXPECT_FLOAT_EQ(first.total_salary[period], second.total_salary[period],
                        0.0001);
    }
}

TEST(posted_prices_respond_to_realized_sales_after_market_clearing) {
    vector<Pop> pops{Pop(), Pop()};
    vector<Factory*> factories;
    Market market(&pops, &factories);
    Government government(&factories, &pops, 0.0f, &market);
    Factory sold_out(&pops[0], &market, &government, production_config(), 0);
    Factory unsold(&pops[1], &market, &government, production_config(), 0);
    factories = {&sold_out, &unsold};
    sold_out.production = 10;
    sold_out.my_price = 1;
    unsold.production = 10;
    unsold.my_price = 2;
    pops[0].number = 10;
    pops[0].money = 100;
    pops[0].Consumption["Money_for_Food"] = 1;
    pops[1].number = 0;
    market.Stats["Food_value"] = 1;

    market.Update();
    sold_out.CalculatePrice();
    unsold.CalculatePrice();

    EXPECT_FLOAT_EQ(sold_out.stock_sold, 10.0f, 0.0001f);
    EXPECT_FLOAT_EQ(unsold.stock_sold, 0.0f, 0.0001f);
    EXPECT_GT(sold_out.my_price, 1.0f);
    EXPECT_LT(unsold.my_price, 2.0f);
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}

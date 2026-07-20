#include "test_utils.h"
#include "../Agents/Factory.h"
#include "../Agents/Pop.h"
#include "../Agents/Market.h"
#include "../Agents/Government.h"
#include "../lib/json.hpp"
#include <fstream>

using namespace std;

static nlohmann::json load_json() {
    const char* paths[] = {"../Production_methods.json", "Production_methods.json", "../../Production_methods.json"};
    for (const char* p : paths) {
        ifstream f(p);
        if (f.good()) {
            nlohmann::json j;
            f >> j;
            return j;
        }
    }
    return nlohmann::json{};
}

// ==========================================================================
// 1. Constructor tests
// ==========================================================================

TEST(Constructor_Food_type) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    EXPECT_EQ(factory.type, "Food");
}

TEST(Constructor_Cloth_type) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 1);
    EXPECT_EQ(factory.type, "Cloth");
}

TEST(Constructor_Food_Efficiency_parameter) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    EXPECT_FLOAT_EQ(factory.Efficiency_parameter, 20.0f, 0.0001f);
}

TEST(Constructor_Food_sigma) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    EXPECT_FLOAT_EQ(factory.sigma, 0.8f, 0.0001f);
}

TEST(Constructor_Food_distribution_parameter) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    EXPECT_FLOAT_EQ(factory.distribution_parameter, 0.35f, 0.0001f);
}

TEST(Constructor_Food_sigma_sostitution) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    float expected = (0.8f - 1.0f) / 0.8f;
    EXPECT_FLOAT_EQ(factory.sigma_sostitution, expected, 0.0001f);
}

TEST(Constructor_Cloth_Efficiency_parameter) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 1);
    EXPECT_FLOAT_EQ(factory.Efficiency_parameter, 2.0f, 0.0001f);
}

TEST(Constructor_Cloth_sigma) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 1);
    EXPECT_FLOAT_EQ(factory.sigma, 0.8f, 0.0001f);
}

TEST(Constructor_Cloth_distribution_parameter) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 1);
    EXPECT_FLOAT_EQ(factory.distribution_parameter, 0.35f, 0.0001f);
}

TEST(Constructor_Cloth_sigma_sostitution) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 1);
    float expected = (0.8f - 1.0f) / 0.8f;
    EXPECT_FLOAT_EQ(factory.sigma_sostitution, expected, 0.0001f);
}

TEST(Constructor_default_values) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    EXPECT_FLOAT_EQ(factory.investment, 1.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.cash, 1000.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.money_gained, 0.0f, 0.0001f);
    EXPECT_EQ(factory.production, 0);
}

// ==========================================================================
// 2. Troughput() tests
// ==========================================================================

TEST(Troughput_investment_1) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 1;
    float expected = log(11.0f) / log(10000.0f);
    EXPECT_FLOAT_EQ(factory.Troughput(), expected, 0.0001f);
}

TEST(Troughput_investment_100) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 100;
    float expected = log(110.0f) / log(10000.0f);
    EXPECT_FLOAT_EQ(factory.Troughput(), expected, 0.0001f);
}

TEST(Troughput_investment_10000) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 10000;
    float expected = log(10010.0f) / log(10000.0f);
    EXPECT_FLOAT_EQ(factory.Troughput(), expected, 0.0001f);
}

TEST(Troughput_increases_with_investment) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 1;
    float t1 = factory.Troughput();
    factory.investment = 100;
    float t2 = factory.Troughput();
    factory.investment = 10000;
    float t3 = factory.Troughput();
    EXPECT_LT(t1, t2);
    EXPECT_LT(t2, t3);
}

// ==========================================================================
// 3. Production() tests
// ==========================================================================

TEST(Production_normal_Food) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 100;
    float prev_cash = factory.cash;
    float prev_produced = market.Stats["Food_produced"];

    factory.Production();

    float expected_prod = 20.0f * 100.0f;
    EXPECT_FLOAT_EQ(float(factory.production), expected_prod, 0.5f);
    float expected_money = expected_prod * market.Stats["Food_value"];
    EXPECT_FLOAT_EQ(factory.money_gained, expected_money, 0.5f);
    EXPECT_FLOAT_EQ(market.Stats["Food_produced"], prev_produced + factory.production, 0.5f);
    EXPECT_FLOAT_EQ(factory.cash, prev_cash + factory.money_gained, 0.5f);
}

TEST(Production_normal_Cloth) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 1);
    factory.investment = 100;
    float prev_cash = factory.cash;
    float prev_produced = market.Stats["Cloth_produced"];

    factory.Production();

    float expected_prod = 2.0f * 100.0f;
    EXPECT_FLOAT_EQ(float(factory.production), expected_prod, 0.5f);
    float expected_money = expected_prod * market.Stats["Cloth_value"];
    EXPECT_FLOAT_EQ(factory.money_gained, expected_money, 0.5f);
    EXPECT_FLOAT_EQ(market.Stats["Cloth_produced"], prev_produced + factory.production, 0.5f);
    EXPECT_FLOAT_EQ(factory.cash, prev_cash + factory.money_gained, 0.5f);
}

TEST(Production_workers_zero_clamped_to_one) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 0;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 100;

    factory.Production();

    EXPECT_GT(float(factory.production), 0);
    EXPECT_TRUE(!std::isnan(float(factory.production)));
}

TEST(Production_investment_zero_clamped_to_one) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 0;

    factory.Production();

    EXPECT_GT(float(factory.production), 0);
    EXPECT_TRUE(!std::isnan(float(factory.production)));
}

TEST(Production_very_large_workers_and_investment) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 10000;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 10000;

    factory.Production();

    float expected = 20.0f * 10000.0f;
    EXPECT_FLOAT_EQ(float(factory.production), expected, 10.0f);
    EXPECT_GT(factory.money_gained, 0.0f);
    EXPECT_GT(factory.cash, 1000.0f);
}

TEST(Production_money_gained_equals_production_times_market_value) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 50;
    Market market(nullptr);
    market.Stats["Food_value"] = 150.0f;
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 50;

    factory.Production();

    EXPECT_FLOAT_EQ(factory.money_gained, float(factory.production) * 150.0f, 0.5f);
}

TEST(Production_stats_produced_increases) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    market.Stats["Food_produced"] = 50.0f;
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 100;

    factory.Production();

    EXPECT_GT(market.Stats["Food_produced"], 50.0f);
}

TEST(Production_cash_increases_by_money_gained) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 100;
    float cash_before = factory.cash;

    factory.Production();

    EXPECT_FLOAT_EQ(factory.cash, cash_before + factory.money_gained, 0.5f);
}

TEST(Production_accumulates_stats_over_multiple_calls) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 100;

    factory.Production();
    float prod1 = float(factory.production);
    float produced1 = market.Stats["Food_produced"];

    factory.Production();
    float prod2 = float(factory.production);
    float produced2 = market.Stats["Food_produced"];

    EXPECT_FLOAT_EQ(produced2, produced1 + prod2, 0.5f);
    EXPECT_GT(produced2, produced1);
}

// ==========================================================================
// 4. Mean_cash_12_months() tests
// ==========================================================================

TEST(Mean_cash_12_months_first_call_returns_money_gained) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.money_gained = 500.0f;

    float mean = factory.Mean_cash_12_months();

    EXPECT_FLOAT_EQ(mean, 500.0f, 0.0001f);
    EXPECT_EQ(factory.cash_12_months.size(), size_t(1));
}

TEST(Mean_cash_12_months_rolling_window_builds_up) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);

    factory.money_gained = 100;
    float m1 = factory.Mean_cash_12_months();
    factory.money_gained = 200;
    float m2 = factory.Mean_cash_12_months();
    factory.money_gained = 300;
    float m3 = factory.Mean_cash_12_months();
    factory.money_gained = 400;
    float m4 = factory.Mean_cash_12_months();
    factory.money_gained = 500;
    float m5 = factory.Mean_cash_12_months();

    EXPECT_FLOAT_EQ(m1, 100.0f, 0.0001f);
    EXPECT_FLOAT_EQ(m2, 150.0f, 0.0001f);
    EXPECT_FLOAT_EQ(m3, 200.0f, 0.0001f);
    EXPECT_FLOAT_EQ(m4, 250.0f, 0.0001f);
    EXPECT_FLOAT_EQ(m5, 300.0f, 0.0001f);
    EXPECT_EQ(factory.cash_12_months.size(), size_t(5));
}

TEST(Mean_cash_12_months_after_12_calls_rolling_average) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);

    for (int i = 0; i < 12; i++) {
        factory.money_gained = 100.0f * float(i + 1);
        factory.Mean_cash_12_months();
    }

    EXPECT_EQ(factory.cash_12_months.size(), size_t(12));
    EXPECT_FLOAT_EQ(factory.cash_12_months[0], 100.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.cash_12_months[11], 1200.0f, 0.0001f);

    factory.money_gained = 1300.0f;
    float mean = factory.Mean_cash_12_months();

    EXPECT_FLOAT_EQ(mean, 750.0f, 0.0001f);
    EXPECT_EQ(factory.cash_12_months.size(), size_t(12));
    EXPECT_FLOAT_EQ(factory.cash_12_months[0], 200.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.cash_12_months[11], 1300.0f, 0.0001f);
}

TEST(Mean_cash_12_months_size_never_exceeds_12) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);

    for (int i = 0; i < 20; i++) {
        factory.money_gained = 100.0f;
        factory.Mean_cash_12_months();
    }

    EXPECT_EQ(factory.cash_12_months.size(), size_t(12));
}

TEST(Mean_cash_12_months_rolling_window_correct_after_20_calls) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);

    for (int i = 0; i < 12; i++) {
        factory.money_gained = 100.0f;
        factory.Mean_cash_12_months();
    }

    for (int i = 0; i < 8; i++) {
        factory.money_gained = 200.0f;
        factory.Mean_cash_12_months();
    }

    float mean = factory.Mean_cash_12_months();
    float expected = (3.0f * 100.0f + 9.0f * 200.0f) / 12.0f;

    EXPECT_FLOAT_EQ(mean, expected, 0.0001f);
}

TEST(Mean_cash_12_months_handles_zero_money_gained) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);

    factory.money_gained = 100;
    factory.Mean_cash_12_months();
    factory.money_gained = 0;
    float mean = factory.Mean_cash_12_months();

    EXPECT_FLOAT_EQ(mean, 50.0f, 0.0001f);
}

// ==========================================================================
// 5. Salary() tests
// ==========================================================================

TEST(Salary_cash_negative_clamped_to_zero) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.cash = -500.0f;
    factory.money_gained = 1000.0f;

    factory.Salary();

    EXPECT_FLOAT_EQ(factory.salary, 1000.0f, 0.0001f);
}

TEST(Salary_equals_mean_cash_12_months) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.cash = 5000.0f;
    factory.money_gained = 100;
    factory.Mean_cash_12_months();
    factory.money_gained = 200;
    factory.Mean_cash_12_months();
    factory.money_gained = 300;
    factory.Mean_cash_12_months();

    float cash_before = factory.cash;
    factory.Salary();

    EXPECT_FLOAT_EQ(factory.salary, 225.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.cash, cash_before - factory.salary, 0.0001f);
}

TEST(Salary_negative_salary_sets_to_money_gained) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.cash = 5000.0f;
    factory.money_gained = -500.0f;

    factory.Salary();

    EXPECT_FLOAT_EQ(factory.salary, -500.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.cash, 5500.0f, 0.0001f);
}

TEST(Salary_cash_decreases_by_salary) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.cash = 10000.0f;
    factory.money_gained = 1000.0f;

    float cash_before = factory.cash;
    factory.Salary();

    EXPECT_FLOAT_EQ(factory.cash, cash_before - factory.salary, 0.0001f);
}

TEST(Salary_repeated_calls_with_negative_cash) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.cash = -100.0f;
    factory.money_gained = 50.0f;

    factory.Salary();

    EXPECT_FLOAT_EQ(factory.salary, 50.0f, 0.0001f);

    factory.money_gained = 30.0f;
    factory.Salary();

    EXPECT_FLOAT_EQ(factory.salary, 40.0f, 0.0001f);
}

// ==========================================================================
// 6. salary_() tests
// ==========================================================================

TEST(salary_returns_current_salary) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.salary = 2500.0f;

    EXPECT_FLOAT_EQ(factory.salary_(), 2500.0f, 0.0001f);
}

TEST(salary_returns_value_set_by_Salary) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.cash = 5000.0f;
    factory.money_gained = 700.0f;

    factory.Salary();

    EXPECT_FLOAT_EQ(factory.salary_(), factory.salary, 0.0001f);
}

TEST(salary_returns_default_salary_before_any_Salary_call) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);

    EXPECT_FLOAT_EQ(factory.salary_(), 1000.0f, 0.0001f);
}

// ==========================================================================
// 7. Production CES edge cases
// ==========================================================================

TEST(Production_ces_no_NaN_Food) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 100;

    factory.Production();

    EXPECT_TRUE(!std::isnan(float(factory.production)));
    EXPECT_TRUE(!std::isnan(factory.money_gained));
    EXPECT_TRUE(!std::isnan(factory.cash));
}

TEST(Production_ces_no_NaN_Cloth) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 1);
    factory.investment = 100;

    factory.Production();

    EXPECT_TRUE(!std::isnan(float(factory.production)));
    EXPECT_TRUE(!std::isnan(factory.money_gained));
    EXPECT_TRUE(!std::isnan(factory.cash));
}

TEST(Production_ces_no_NaN_edge_cases) {
    nlohmann::json j = load_json();
    int worker_values[] = {1, 10, 100, 1000, 10000};
    int investment_values[] = {1, 10, 100, 1000, 10000};

    for (int w : worker_values) {
        for (int inv : investment_values) {
            Pop pop;
            pop.number = w;
            Market market(nullptr);
            Government gov;
            Factory factory(&pop, &market, &gov, j, 0);
            factory.investment = float(inv);

            factory.Production();

            EXPECT_TRUE(!std::isnan(float(factory.production)));
            EXPECT_TRUE(!std::isnan(factory.money_gained));
            EXPECT_FALSE(std::isinf(float(factory.production)));
            EXPECT_GE(float(factory.production), 0.0f);
        }
    }
}

TEST(Production_ces_Food_positive_output) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 50;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 50;

    factory.Production();

    EXPECT_GT(float(factory.production), 0);
    EXPECT_GT(factory.money_gained, 0.0f);
}

TEST(Production_ces_Cloth_positive_output) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 50;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 1);
    factory.investment = 50;

    factory.Production();

    EXPECT_GT(float(factory.production), 0);
    EXPECT_GT(factory.money_gained, 0.0f);
}

TEST(Production_ces_Food_vs_Cloth_ratio) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 100;
    Market market(nullptr);
    Government gov;

    Factory food_factory(&pop, &market, &gov, j, 0);
    Factory cloth_factory(&pop, &market, &gov, j, 1);
    food_factory.investment = 100;
    cloth_factory.investment = 100;

    food_factory.Production();
    cloth_factory.Production();

    float ratio = float(food_factory.production) / float(cloth_factory.production);
    EXPECT_FLOAT_EQ(ratio, 10.0f, 0.1f);
}

TEST(Production_ces_asymmetric_workers_and_investment) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 1000;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 10;

    factory.Production();

    EXPECT_GT(float(factory.production), 0);
    EXPECT_TRUE(!std::isnan(float(factory.production)));
    EXPECT_LT(float(factory.production), 1000.0f * 20.0f);
}

TEST(Production_ces_minimum_workers_and_investment) {
    nlohmann::json j = load_json();
    Pop pop;
    pop.number = 0;
    Market market(nullptr);
    Government gov;
    Factory factory(&pop, &market, &gov, j, 0);
    factory.investment = 0;

    factory.Production();

    EXPECT_GT(float(factory.production), 0);
    EXPECT_TRUE(!std::isnan(float(factory.production)));
    EXPECT_TRUE(!std::isnan(factory.money_gained));
}

// ==========================================================================

int main() {
    RUN_ALL_TESTS();
    return 0;
}

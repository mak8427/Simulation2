#include "test_utils.h"
#include "../Agents/Pop.h"
#include "../Agents/Factory.h"
#include "../Agents/Market.h"
#include "../Agents/Government.h"
#include "../lib/json.hpp"
#include <vector>
#include <map>
#include <cmath>
#include <string>

static nlohmann::json make_config(float reserve_constant = 0.5f, float reproduction_rate = 0.02f) {
    return nlohmann::json{{"reserve_constant", reserve_constant}, {"reproduction_rate", reproduction_rate}};
}

TEST(default_constructor) {
    Pop p;
    EXPECT_EQ(p.number, 100);
    EXPECT_EQ(p.money, 10000.0f);
    EXPECT_EQ(p.personal_loan, 0.0f);
    EXPECT_EQ(p.months_with_food, 0);
    EXPECT_EQ(p.reserve_constant, 0.0f);
    EXPECT_EQ(p.Goods["Food"], 0.0f);
    EXPECT_EQ(p.Goods["Cloth"], 0.0f);
    EXPECT_EQ(p.Consumption["Food_supply"], 1.0f);
    EXPECT_FLOAT_EQ(p.Consumption["Food_importance"], 0.5f, 1e-6f);
    EXPECT_FLOAT_EQ(p.Consumption["Cloth_importance"], 0.5f, 1e-6f);
    EXPECT_EQ(p.Consumption["Money_for_Food"], 0.0f);
    EXPECT_EQ(p.Consumption["Money_for_Cloth"], 0.0f);
}

TEST(parameterized_constructor) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.15f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory_ptrs.push_back(&factory);

    nlohmann::json j = make_config(0.75f, 0.03f);

    Pop p(250, 999, 42, &factory, &market, &gov, j);
    EXPECT_EQ(p.number, 250);
    EXPECT_EQ(p.money, 10000.0f);
    EXPECT_EQ(p.Goods["Food"], 42.0f);
    EXPECT_EQ(p.factory, &factory);
    EXPECT_EQ(p.markets, &market);
    EXPECT_EQ(p.gov, &gov);
    EXPECT_FLOAT_EQ(p.reserve_constant, 0.75f, 1e-6f);
    EXPECT_FLOAT_EQ(p.reproduction_rate, 0.03f, 1e-6f);
}

TEST(food_consumed_normal) {
    Pop p;
    p.number = 100;
    EXPECT_EQ(p.food_consumed(), 100.0f);
}

TEST(food_consumed_zero_pop) {
    Pop p;
    p.number = 0;
    EXPECT_EQ(p.food_consumed(), 1.0f);
}

TEST(food_consumed_various) {
    Pop p;
    p.number = 1;
    EXPECT_EQ(p.food_consumed(), 1.0f);
    p.number = 100;
    EXPECT_EQ(p.food_consumed(), 100.0f);
    p.number = 1000;
    EXPECT_EQ(p.food_consumed(), 1000.0f);
}

TEST(cloth_used_sufficient) {
    Pop p;
    p.number = 120;
    p.Goods["Cloth"] = 50.0f;
    float used = p.cloth_used();
    EXPECT_EQ(used, 0.0f);
}

TEST(cloth_used_deficit) {
    Pop p;
    p.number = 120;
    p.Goods["Cloth"] = 3.0f;
    float used = p.cloth_used();
    EXPECT_EQ(used, 7.0f);
}

TEST(cloth_used_zero_cloth) {
    Pop p;
    p.number = 120;
    p.Goods["Cloth"] = 0.0f;
    float used = p.cloth_used();
    EXPECT_EQ(used, 10.0f);
}

TEST(cloth_used_rounding) {
    Pop p;
    p.number = 1;
    p.Goods["Cloth"] = 0.0f;
    float used = p.cloth_used();
    EXPECT_EQ(used, 1.0f);
}

TEST(food_variation) {
    Pop p;
    p.number = 100;
    p.Goods["Food"] = 200.0f;
    p.Goods["Cloth"] = 30.0f;
    p.Food_variation();
    EXPECT_EQ(p.Goods["Food"], 100.0f);
    EXPECT_EQ(p.Goods["Cloth"], 22.0f);
}

TEST(pop_variation_growth) {
    Pop p;
    p.number = 100;
    p.Goods["Food"] = 50.0f;
    int old_number = p.number;
    p.Pop_variation();
    EXPECT_GT(p.number, old_number);
}

TEST(pop_variation_starvation) {
    Pop p;
    p.number = 100;
    p.Goods["Food"] = -50.0f;
    int old_number = p.number;
    p.Pop_variation();
    EXPECT_LT(p.number, old_number);
    EXPECT_GT(p.number, 0);
}

TEST(pop_variation_starvation_clamp_to_one) {
    Pop p;
    p.number = 2;
    p.Goods["Food"] = -200.0f;
    p.Pop_variation();
    EXPECT_EQ(p.number, 1);
}

TEST(pop_variation_cloth_floor) {
    Pop p;
    p.number = 5;
    p.Goods["Cloth"] = -5.0f;
    p.Goods["Food"] = 50.0f;
    p.Pop_variation();
    EXPECT_EQ(p.Goods["Cloth"], 0.0f);
}

TEST(sol_food_positive) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.0f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, 50, &factory, &market, &gov, make_config());
    p.Consumption["Food_importance"] = 0.5f;
    int old_months = p.months_with_food;

    p.SOL();

    EXPECT_EQ(p.months_with_food, old_months + 1);
    EXPECT_EQ(p.Consumption["Food_supply"], 1.0f);
    EXPECT_LT(p.Consumption["Food_importance"], 0.5f);
}

TEST(sol_food_negative) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.0f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, -1, &factory, &market, &gov, make_config());
    p.Consumption["Food_importance"] = 0.5f;
    p.months_with_food = 10;

    p.SOL();

    EXPECT_EQ(p.months_with_food, 0);
    EXPECT_GT(p.Consumption["Food_importance"], 0.5f);
    EXPECT_EQ(p.Goods["Food"], 0.0f);
    EXPECT_LT(p.Consumption["Food_supply"], 0.0f);
}

TEST(sol_food_zero_months_ge_12) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.0f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, 0, &factory, &market, &gov, make_config());
    p.months_with_food = 12;

    p.SOL();

    EXPECT_EQ(p.months_with_food, 13);
    EXPECT_EQ(p.Consumption["Food_supply"], 1.0f);
}

TEST(sol_food_zero_months_lt_12) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.0f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, 0, &factory, &market, &gov, make_config());
    p.Consumption["Food_supply"] = 999.0f;
    p.months_with_food = 5;

    p.SOL();

    EXPECT_EQ(p.months_with_food, 5);
    EXPECT_EQ(p.Consumption["Food_supply"], 999.0f);
}

TEST(sol_personal_loan_set) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.0f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory.salary = 1000.0f;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, -1, &factory, &market, &gov, make_config());
    p.Consumption["Food_importance"] = 0.99f;
    p.money = 10000.0f;

    p.SOL();

    EXPECT_GT(p.Consumption["Food_importance"], 0.99f);
    EXPECT_GT(p.personal_loan, 0.0f);
}

TEST(sol_food_importance_clamp_when_poor) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.0f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory.salary = 1000.0f;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, -1, &factory, &market, &gov, make_config());
    p.Consumption["Food_importance"] = 0.99f;
    p.money = 0.5f;

    p.SOL();

    EXPECT_EQ(p.Consumption["Food_importance"], 0.98f);
    EXPECT_EQ(p.personal_loan, 0.0f);
}

TEST(sol_money_for_food_and_cloth_set) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.0f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory.salary = 1000.0f;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, 50, &factory, &market, &gov, make_config());
    p.Goods["Cloth"] = 0.0f;
    p.Consumption["Food_importance"] = 0.4f;

    p.SOL();

    EXPECT_GT(p.Consumption["Money_for_Food"], 0.0f);
    EXPECT_GT(p.Consumption["Money_for_Cloth"], 0.0f);
}

TEST(sol_cloth_importance_min) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.0f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory.salary = 1000.0f;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, -1, &factory, &market, &gov, make_config());
    p.Consumption["Food_importance"] = 0.99f;
    p.money = 0.0f;

    p.SOL();

    EXPECT_EQ(p.Consumption["Cloth_importance"], 0.01f);
}

TEST(sol_cloth_demand_zero) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.0f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory.salary = 1000.0f;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, 50, &factory, &market, &gov, make_config());
    p.Goods["Cloth"] = 999.0f;
    p.Consumption["Food_importance"] = 0.5f;

    p.SOL();

    EXPECT_EQ(p.Consumption["Money_for_Cloth"], 0.0f);
}

TEST(money_salary_no_tax) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.0f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory.salary = 1000.0f;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, 50, &factory, &market, &gov, make_config());
    float old_money = p.money;
    float old_reserve = gov.reserve;

    p.Money();

    EXPECT_EQ(p.money, old_money + 1000.0f);
    EXPECT_EQ(gov.reserve, old_reserve);
}

TEST(money_salary_with_tax) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.2f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory.salary = 1000.0f;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, 50, &factory, &market, &gov, make_config());
    float old_money = p.money;
    float old_reserve = gov.reserve;

    p.Money();

    EXPECT_EQ(p.money, old_money + 800.0f);
    EXPECT_EQ(gov.reserve, old_reserve + 200.0f);
}

TEST(edge_zero_population) {
    Pop p;
    p.number = 0;
    EXPECT_EQ(p.food_consumed(), 1.0f);

    p.Goods["Food"] = 0.0f;
    p.Goods["Cloth"] = 0.0f;
    p.Food_variation();
    EXPECT_LT(p.Goods["Food"], 0.0f);

    float needed = std::max(1.0f, round(0.0f / 12.0f));
    EXPECT_EQ(needed, 1.0f);
}

TEST(edge_large_population) {
    Pop p;
    p.number = 1000000;
    EXPECT_EQ(p.food_consumed(), 1000000.0f);

    p.Goods["Food"] = 2000000.0f;
    p.Goods["Cloth"] = 100000.0f;
    p.Food_variation();
    EXPECT_EQ(p.Goods["Food"], 1000000.0f);

    p.Pop_variation();
    EXPECT_GT(p.number, 0);
}

TEST(edge_negative_food_supply) {
    Pop p;
    p.number = 100;
    p.Goods["Food"] = -50.0f;
    p.Goods["Cloth"] = 0.0f;

    int old_number = p.number;
    p.Pop_variation();
    EXPECT_LT(p.number, old_number);
    EXPECT_EQ(p.Goods["Cloth"], 0.0f);
}

TEST(edge_all_money_spent) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.0f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory.salary = 1000.0f;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, 50, &factory, &market, &gov, make_config());
    p.money = 0.0f;
    p.Consumption["Food_importance"] = 0.5f;

    p.SOL();
    EXPECT_GE(p.money, 0.0f);

    p.Money();
    EXPECT_GT(p.money, 0.0f);
}

TEST(edge_personal_loan_persists) {
    std::vector<Pop> pops;
    std::vector<Factory*> factory_ptrs;
    Factory factory;
    Market market(&pops);
    Government gov(&factory_ptrs, &pops, 0.0f, &market);
    factory.workers = nullptr;
    factory.gov = &gov;
    factory.markets = &market;
    factory.salary = 1000.0f;
    factory_ptrs.push_back(&factory);

    Pop p(100, 0, -1, &factory, &market, &gov, make_config());
    p.Consumption["Food_importance"] = 0.99f;
    p.money = 10000.0f;

    p.SOL();
    float loan_after_first = p.personal_loan;
    EXPECT_GT(loan_after_first, 0.0f);

    p.Goods["Food"] = -1.0f;
    p.Consumption["Food_importance"] = 0.99f;
    p.SOL();
    float loan_after_second = p.personal_loan;
    EXPECT_GT(loan_after_second, 0.0f);
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}

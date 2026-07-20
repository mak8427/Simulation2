#include "test_utils.h"
#include "../Agents/Factory.h"
#include "../Agents/Government.h"
#include "../Agents/Market.h"
#include "../Agents/Pop.h"

#include <cmath>
#include <random>
#include <vector>

using namespace std;

static nlohmann::json pop_config(float reproduction_rate = 0.02f) {
    return {{"reserve_constant", 0.5f},
            {"reproduction_rate", reproduction_rate}};
}

static int paper_population_next(int population, double carrying_capacity,
                                 double growth_rate, double dt = 1.0) {
    if (population <= 0 || carrying_capacity <= 0) return 0;
    double next = carrying_capacity /
        (1.0 + (carrying_capacity / population - 1.0) *
                   std::exp(-growth_rate * dt));
    return static_cast<int>(std::lround(next));
}

struct PopContext {
    vector<Pop> pops;
    vector<Factory*> factories;
    Market market{&pops, &factories};
    Government government{&factories, &pops, 0.0f, &market};
    Factory factory;

    PopContext() {
        factory.gov = &government;
        factory.markets = &market;
        factory.worker_count = 1;
        factory.workforce = 100;
        factories.push_back(&factory);
    }
};

TEST(default_constructor_has_safe_zero_state_for_links_and_growth_rate) {
    Pop pop;
    EXPECT_EQ(pop.number, 100);
    EXPECT_FLOAT_EQ(pop.money, 10000.0, 0.0001);
    EXPECT_FLOAT_EQ(pop.reproduction_rate, 0.0f, 0.0001f);
    EXPECT_EQ(pop.factory, nullptr);
    EXPECT_EQ(pop.markets, nullptr);
    EXPECT_EQ(pop.gov, nullptr);
}

TEST(parameterized_constructor_honors_every_inserted_value) {
    PopContext context;
    Pop pop(250, 999, 42, &context.factory, &context.market,
            &context.government, pop_config(0.03f));

    EXPECT_EQ(pop.number, 250);
    EXPECT_FLOAT_EQ(pop.money, 999.0, 0.0001);
    EXPECT_FLOAT_EQ(pop.Goods["Food"], 42.0f, 0.0001f);
    EXPECT_EQ(pop.factory, &context.factory);
    EXPECT_EQ(pop.markets, &context.market);
    EXPECT_EQ(pop.gov, &context.government);
    EXPECT_FLOAT_EQ(pop.reserve_constant, 0.5f, 0.0001f);
    EXPECT_FLOAT_EQ(pop.reproduction_rate, 0.03f, 0.0001f);
}

TEST(food_demand_is_one_per_person_and_zero_when_extinct) {
    Pop pop;
    pop.number = 123;
    EXPECT_FLOAT_EQ(pop.food_consumed(), 123.0f, 0.0001f);
    pop.number = 0;
    EXPECT_FLOAT_EQ(pop.food_consumed(), 0.0f, 0.0001f);
}

TEST(cloth_demand_uses_rounded_monthly_need_and_existing_stock) {
    Pop pop;
    pop.number = 120;
    pop.Goods["Cloth"] = 3;
    EXPECT_FLOAT_EQ(pop.cloth_used(), 7.0f, 0.0001f);
    pop.Goods["Cloth"] = 10;
    EXPECT_FLOAT_EQ(pop.cloth_used(), 0.0f, 0.0001f);
    pop.number = 0;
    pop.Goods["Cloth"] = 0;
    EXPECT_FLOAT_EQ(pop.cloth_used(), 0.0f, 0.0001f);
}

TEST(consumption_reduces_food_and_cloth_by_population_need) {
    Pop pop;
    pop.number = 100;
    pop.Goods["Food"] = 200;
    pop.Goods["Cloth"] = 30;
    pop.Food_variation();
    EXPECT_FLOAT_EQ(pop.Goods["Food"], 100.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pop.Goods["Cloth"], 22.0f, 0.0001f);
}

TEST(paper_equation_is_stationary_when_capacity_equals_population) {
    Pop pop;
    pop.number = 100;
    pop.reproduction_rate = 0.2f;
    mt19937 gen(7);

    pop.Pop_variation(gen, 100.0f);

    EXPECT_EQ(pop.number, 100);
    EXPECT_EQ(pop.number, paper_population_next(100, 100, 0.2));
}

TEST(implicit_capacity_uses_food_stock_before_period_consumption) {
    Pop pop;
    pop.number = 100;
    pop.reproduction_rate = 0.2f;
    pop.Goods["Food"] = 100;
    mt19937 gen(7);

    pop.Food_variation();
    EXPECT_FLOAT_EQ(pop.Goods["Food"], 0.0f, 0.0001f);
    pop.Pop_variation(gen);

    EXPECT_EQ(pop.number, 100);
}

TEST(paper_equation_grows_toward_capacity_when_capacity_is_higher) {
    Pop pop;
    pop.number = 100;
    pop.reproduction_rate = 0.2f;
    mt19937 gen(7);

    pop.Pop_variation(gen, 200.0f);

    EXPECT_EQ(pop.number, paper_population_next(100, 200, 0.2));
    EXPECT_GT(pop.number, 100);
    EXPECT_LT(pop.number, 200);
}

TEST(paper_equation_declines_gradually_toward_positive_lower_capacity) {
    Pop pop;
    pop.number = 100;
    pop.reproduction_rate = 0.2f;
    mt19937 gen(7);

    pop.Pop_variation(gen, 50.0f);

    EXPECT_EQ(pop.number, paper_population_next(100, 50, 0.2));
    EXPECT_LT(pop.number, 100);
    EXPECT_GT(pop.number, 50);
}

TEST(zero_capacity_is_the_extinction_idealization) {
    Pop pop;
    pop.number = 100;
    pop.reproduction_rate = 0.2f;
    mt19937 gen(7);

    pop.Pop_variation(gen, 0.0f);

    EXPECT_EQ(pop.number, 0);
    EXPECT_FLOAT_EQ(pop.Goods["Food"], 0.0f, 0.0001f);
}

TEST(zero_growth_rate_leaves_population_unchanged_for_any_positive_capacity) {
    Pop below_capacity;
    below_capacity.number = 100;
    below_capacity.reproduction_rate = 0.0f;
    Pop above_capacity = below_capacity;
    mt19937 gen(7);

    below_capacity.Pop_variation(gen, 200.0f);
    above_capacity.Pop_variation(gen, 50.0f);

    EXPECT_EQ(below_capacity.number, 100);
    EXPECT_EQ(above_capacity.number, 100);
}

TEST(high_growth_rate_converges_to_capacity_in_one_discrete_step) {
    Pop pop;
    pop.number = 100;
    pop.reproduction_rate = 50.0f;
    mt19937 gen(7);

    pop.Pop_variation(gen, 240.0f);

    EXPECT_EQ(pop.number, paper_population_next(100, 240, 50));
    EXPECT_EQ(pop.number, 240);
}

TEST(extinct_population_remains_extinct_even_with_positive_capacity) {
    Pop pop;
    pop.number = 0;
    pop.reproduction_rate = 0.2f;
    pop.Goods["Food"] = 100;
    mt19937 gen(7);

    pop.Pop_variation(gen, 100.0f);

    EXPECT_EQ(pop.number, 0);
    EXPECT_FLOAT_EQ(pop.Goods["Food"], 0.0f, 0.0001f);
}

TEST(total_food_capacity_is_allocated_proportionally_across_pop_groups) {
    vector<Pop> pops(2);
    pops[0].number = 60;
    pops[1].number = 180;
    pops[0].reproduction_rate = std::log(2.0f);
    pops[1].reproduction_rate = std::log(2.0f);
    const float total_capacity = 120.0f;
    mt19937 gen(7);

    Pop::Population_variation(pops, gen, total_capacity);

    EXPECT_EQ(pops[0].number + pops[1].number,
              paper_population_next(240, total_capacity, std::log(2.0)));
    EXPECT_EQ(pops[1].number, 3 * pops[0].number);
}

TEST(aggregate_rounding_does_not_create_a_floor_per_pop_group) {
    vector<Pop> pops(201);
    for (Pop& pop : pops) {
        pop.number = 7;
        pop.reproduction_rate = 0.05f;
    }
    const int initial_population = 7 * static_cast<int>(pops.size());
    const float total_capacity = 566;
    mt19937 gen(7);

    Pop::Population_variation(pops, gen, total_capacity);

    int actual_population = 0;
    for (const Pop& pop : pops) actual_population += pop.number;
    EXPECT_EQ(actual_population,
              paper_population_next(initial_population, total_capacity, 0.05));
    EXPECT_LT(actual_population, initial_population);
}

TEST(population_growth_clears_depleted_cloth_stock) {
    Pop pop;
    pop.number = 100;
    pop.reproduction_rate = 0.02f;
    pop.Goods["Food"] = 1;
    pop.Goods["Cloth"] = -4;
    mt19937 gen(11);

    pop.Pop_variation(gen, 150.0f);
    EXPECT_FLOAT_EQ(pop.Goods["Cloth"], 0.0f, 0.0001f);
}

TEST(sol_moves_priority_from_food_when_stocked) {
    PopContext context;
    Pop pop(100, 1200, 50, &context.factory, &context.market,
            &context.government, pop_config());
    pop.Consumption["Food_importance"] = 0.5f;
    mt19937 gen(5);

    pop.SOL(gen);
    EXPECT_EQ(pop.months_with_food, 1);
    EXPECT_LT(pop.Consumption["Food_importance"], 0.5f);
    EXPECT_FLOAT_EQ(pop.Consumption["Food_supply"], 1.0f, 0.0001f);
}

TEST(sol_moves_priority_to_food_when_starving) {
    PopContext context;
    Pop pop(100, 1200, -10, &context.factory, &context.market,
            &context.government, pop_config());
    pop.Consumption["Food_importance"] = 0.5f;
    pop.months_with_food = 4;
    mt19937 gen(5);

    pop.SOL(gen);
    EXPECT_EQ(pop.months_with_food, 0);
    EXPECT_GT(pop.Consumption["Food_importance"], 0.5f);
    EXPECT_LT(pop.Consumption["Food_supply"], 0.0f);
    EXPECT_FLOAT_EQ(pop.Goods["Food"], 0.0f, 0.0001f);
}

TEST(sol_extinct_pop_has_no_budget_or_personal_loan) {
    Pop pop;
    pop.number = 0;
    pop.personal_loan = 50;
    pop.Consumption["Money_for_Food"] = 20;
    pop.Consumption["Money_for_Cloth"] = 10;
    mt19937 gen(5);

    pop.SOL(gen);
    EXPECT_FLOAT_EQ(pop.personal_loan, 0.0, 0.0001);
    EXPECT_FLOAT_EQ(pop.Consumption["Money_for_Food"], 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pop.Consumption["Money_for_Cloth"], 0.0f, 0.0001f);
}

TEST(unemployed_pop_bids_from_savings_without_dereferencing_government) {
    Pop pop;
    pop.number = 100;
    pop.money = 1200;
    pop.Goods["Food"] = 10;
    pop.Goods["Cloth"] = 0;
    pop.Consumption["Food_importance"] = 0.5f;
    mt19937 gen(13);

    pop.SOL(gen);
    EXPECT_GT(pop.Consumption["Money_for_Food"], 0.0f);
    EXPECT_FLOAT_EQ(pop.Consumption["Money_for_Cloth"], 0.0f, 0.0001f);
}

TEST(satisfied_cloth_need_produces_no_cloth_bid) {
    PopContext context;
    context.factory.salary = 1000;
    Pop pop(120, 0, 10, &context.factory, &context.market,
            &context.government, pop_config());
    pop.Goods["Cloth"] = 10;
    mt19937 gen(17);

    pop.SOL(gen);
    EXPECT_FLOAT_EQ(pop.Consumption["Money_for_Cloth"], 0.0f, 0.0001f);
}

TEST(money_transfers_full_salary_without_tax) {
    PopContext context;
    context.factory.salary = 1000;
    context.factory.workforce = 100;
    Pop pop(100, 25, 0, &context.factory, &context.market,
            &context.government, pop_config());

    pop.Money();
    EXPECT_FLOAT_EQ(pop.money, 1025.0, 0.0001);
    EXPECT_FLOAT_EQ(context.government.reserve, 0.0f, 0.0001f);
}

TEST(money_splits_gross_salary_between_worker_and_tax_reserve) {
    PopContext context;
    context.government.tax_rate = 0.2f;
    context.factory.salary = 1000;
    context.factory.workforce = 100;
    Pop pop(100, 25, 0, &context.factory, &context.market,
            &context.government, pop_config());

    pop.Money();
    EXPECT_FLOAT_EQ(pop.money, 825.0, 0.0001);
    EXPECT_FLOAT_EQ(context.government.reserve, 200.0f, 0.0001f);
}

TEST(shared_payroll_is_proportional_to_population) {
    PopContext context;
    context.government.tax_rate = 0.1f;
    context.factory.salary = 200;
    context.factory.worker_count = 2;
    context.factory.workforce = 100;
    Pop small(25, 0, 0, &context.factory, &context.market,
              &context.government, pop_config());
    Pop large(75, 0, 0, &context.factory, &context.market,
              &context.government, pop_config());

    small.Money();
    large.Money();
    EXPECT_FLOAT_EQ(small.money, 45.0, 0.0001);
    EXPECT_FLOAT_EQ(large.money, 135.0, 0.0001);
    EXPECT_FLOAT_EQ(context.government.reserve, 20.0f, 0.0001f);
}

TEST(unemployed_or_zero_workforce_pop_receives_no_salary) {
    PopContext context;
    Pop unemployed(100, 10, 0, nullptr, &context.market,
                   &context.government, pop_config());
    unemployed.Money();
    EXPECT_FLOAT_EQ(unemployed.money, 10.0, 0.0001);

    Pop employed(100, 10, 0, &context.factory, &context.market,
                 &context.government, pop_config());
    context.factory.workforce = 0;
    employed.Money();
    EXPECT_FLOAT_EQ(employed.money, 10.0, 0.0001);
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}

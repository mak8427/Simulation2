#include "test_utils.h"
#include "../Agents/Factory.h"
#include "../Agents/Government.h"
#include "../Agents/Market.h"
#include "../Agents/Pop.h"

#include <cmath>
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

struct FactoryContext {
    vector<Pop> pops{Pop()};
    vector<Factory*> factories;
    Market market{&pops, &factories};
    Government government{&factories, &pops, 0.0f, &market};

    Factory make(int type) {
        return Factory(&pops[0], &market, &government,
                       production_config(), type);
    }
};

TEST(constructor_selects_food_method_and_initial_workforce) {
    FactoryContext context;
    context.pops[0].number = 125;
    Factory factory = context.make(0);

    EXPECT_EQ(factory.type, "Food");
    EXPECT_FLOAT_EQ(factory.Efficiency_parameter, 20.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.sigma, 0.8f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.distribution_parameter, 0.35f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.sigma_sostitution, -0.25f, 0.0001f);
    EXPECT_EQ(factory.workforce, 125);
    EXPECT_EQ(factory.workers, &context.pops[0]);
}

TEST(constructor_selects_cloth_method) {
    FactoryContext context;
    Factory factory = context.make(1);
    EXPECT_EQ(factory.type, "Cloth");
    EXPECT_FLOAT_EQ(factory.Efficiency_parameter, 2.0f, 0.0001f);
}

TEST(default_factory_links_are_null_and_balances_are_finite) {
    Factory factory;
    EXPECT_EQ(factory.workers, nullptr);
    EXPECT_EQ(factory.gov, nullptr);
    EXPECT_EQ(factory.markets, nullptr);
    EXPECT_TRUE(std::isfinite(factory.cash));
    EXPECT_TRUE(std::isfinite(factory.salary));
}

TEST(throughput_increases_with_investment) {
    Factory factory;
    factory.investment = 1;
    float low = factory.Troughput();
    factory.investment = 100;
    float medium = factory.Troughput();
    factory.investment = 10000;
    float high = factory.Troughput();
    EXPECT_LT(low, medium);
    EXPECT_LT(medium, high);
}

TEST(food_production_records_goods_but_does_not_create_money) {
    FactoryContext context;
    Factory factory = context.make(0);
    context.factories.push_back(&factory);
    factory.workforce = 100;
    factory.investment = 100;
    factory.cash = 321;
    factory.money_gained = 47;

    factory.Production();

    EXPECT_GT(factory.production, 0);
    EXPECT_FLOAT_EQ(context.market.Stats["Food_produced"], factory.production,
                    0.0001f);
    EXPECT_FLOAT_EQ(factory.cash, 321.0, 0.0001);
    EXPECT_FLOAT_EQ(factory.money_gained, 47.0, 0.0001);
}

TEST(cloth_production_uses_cloth_statistics_only) {
    FactoryContext context;
    Factory factory = context.make(1);
    factory.workforce = 100;

    factory.Production();

    EXPECT_GT(factory.production, 0);
    EXPECT_FLOAT_EQ(context.market.Stats["Cloth_produced"], factory.production,
                    0.0001f);
    EXPECT_FLOAT_EQ(context.market.Stats["Food_produced"], 0.0f, 0.0001f);
}

TEST(no_workforce_means_no_output_even_with_capital) {
    FactoryContext context;
    Factory factory = context.make(0);
    factory.workforce = 0;
    factory.investment = 10000;
    factory.production = 99;
    factory.stock_sold = 20;

    factory.Production();

    EXPECT_EQ(factory.production, 0);
    EXPECT_FLOAT_EQ(factory.stock_sold, 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(context.market.Stats["Food_produced"], 0.0f, 0.0001f);
}

TEST(zero_investment_with_workers_still_has_finite_ces_output) {
    FactoryContext context;
    Factory factory = context.make(0);
    factory.workforce = 100;
    factory.investment = 0;

    factory.Production();

    EXPECT_GT(factory.production, 0);
    EXPECT_TRUE(std::isfinite(static_cast<float>(factory.production)));
}

TEST(production_statistics_accumulate_across_factories) {
    FactoryContext context;
    Factory first = context.make(0);
    Factory second = context.make(0);
    first.workforce = 25;
    second.workforce = 75;

    first.Production();
    second.Production();
    EXPECT_FLOAT_EQ(context.market.Stats["Food_produced"],
                    first.production + second.production, 0.0001f);
}

TEST(rolling_revenue_mean_uses_only_available_observations) {
    Factory factory;
    factory.money_gained = 10;
    EXPECT_FLOAT_EQ(factory.Mean_cash_12_months(), 10.0, 0.0001);
    factory.money_gained = 30;
    EXPECT_FLOAT_EQ(factory.Mean_cash_12_months(), 20.0, 0.0001);
    factory.money_gained = 50;
    EXPECT_FLOAT_EQ(factory.Mean_cash_12_months(), 30.0, 0.0001);
}

TEST(rolling_revenue_mean_discards_observations_older_than_twelve_periods) {
    Factory factory;
    for (int value = 1; value <= 12; ++value) {
        factory.money_gained = value;
        factory.Mean_cash_12_months();
    }
    factory.money_gained = 13;
    double mean = factory.Mean_cash_12_months();

    EXPECT_EQ(factory.cash_12_months.size(), static_cast<size_t>(12));
    EXPECT_FLOAT_EQ(mean, 7.5, 0.0001);
}

TEST(salary_is_revenue_mean_capped_by_available_cash) {
    Factory factory;
    factory.worker_count = 1;
    factory.workforce = 100;
    factory.money_gained = 80;
    factory.cash = 50;

    factory.Salary();

    EXPECT_FLOAT_EQ(factory.salary, 50.0, 0.0001);
    EXPECT_FLOAT_EQ(factory.cash, 0.0, 0.0001);
}

TEST(negative_revenue_never_produces_a_negative_salary) {
    Factory factory;
    factory.worker_count = 1;
    factory.workforce = 100;
    factory.money_gained = -50;
    factory.cash = 100;

    factory.Salary();

    EXPECT_FLOAT_EQ(factory.salary, 0.0, 0.0001);
    EXPECT_FLOAT_EQ(factory.cash, 100.0, 0.0001);
}

TEST(zero_workers_do_not_run_payroll_or_change_cash) {
    Factory factory;
    factory.worker_count = 0;
    factory.workforce = 0;
    factory.money_gained = 80;
    factory.salary = 25;
    factory.cash = 100;

    factory.Salary();

    EXPECT_FLOAT_EQ(factory.salary, 25.0, 0.0001);
    EXPECT_FLOAT_EQ(factory.cash, 100.0, 0.0001);
}

TEST(salary_accessor_returns_current_salary) {
    Factory factory;
    factory.salary = 73.5;
    EXPECT_FLOAT_EQ(factory.salary_(), 73.5, 0.0001);
}

TEST(sold_out_factory_raises_price_and_unsold_factory_lowers_price) {
    FactoryContext context;
    context.market.Stats["Food_value"] = 10;
    Factory sold_out = context.make(0);
    sold_out.my_price = 10;
    sold_out.production = 100;
    sold_out.stock_sold = 100;
    sold_out.CalculatePrice();
    EXPECT_GT(sold_out.my_price, 10.0f);

    Factory unsold = context.make(0);
    unsold.my_price = 10;
    unsold.production = 100;
    unsold.stock_sold = 0;
    unsold.CalculatePrice();
    EXPECT_LT(unsold.my_price, 10.0f);
}

TEST(price_calculation_records_unit_cost_without_changing_money) {
    FactoryContext context;
    context.market.Stats["Food_value"] = 12;
    Factory factory = context.make(0);
    factory.production = 40;
    factory.stock_sold = 20;
    factory.salary = 100;
    factory.cash = 250;
    factory.money_gained = 30;

    factory.CalculatePrice();

    EXPECT_FLOAT_EQ(factory.unit_cost, 2.5f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.cash, 250.0, 0.0001);
    EXPECT_FLOAT_EQ(factory.money_gained, 30.0, 0.0001);
}

TEST(zero_output_leaves_posted_price_unchanged) {
    FactoryContext context;
    Factory factory = context.make(0);
    factory.production = 0;
    factory.my_price = 8;
    factory.unit_cost = 50;

    factory.CalculatePrice();
    EXPECT_FLOAT_EQ(factory.my_price, 8.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.unit_cost, 0.0f, 0.0001f);
}

TEST(workforce_adjustment_hires_without_creating_people_or_money) {
    FactoryContext context;
    Factory factory = context.make(0);
    context.pops.resize(3);
    factory.workers = &context.pops[0];
    factory.production = 100;
    factory.stock_sold = 100;
    context.pops[0].factory = &factory;
    context.pops[1].number = 40;
    context.pops[1].money = 17;
    context.pops[2].number = 60;
    context.pops[2].money = 23;
    vector<Pop*> unemployed{&context.pops[1], &context.pops[2]};

    factory.AdjustWorkforce(context.pops, unemployed);

    EXPECT_EQ(context.pops[1].factory, &factory);
    EXPECT_EQ(context.pops[2].factory, &factory);
    EXPECT_EQ(context.pops[1].number, 40);
    EXPECT_EQ(context.pops[2].number, 60);
    EXPECT_FLOAT_EQ(context.pops[1].money, 17.0, 0.0001);
    EXPECT_FLOAT_EQ(context.pops[2].money, 23.0, 0.0001);
    EXPECT_TRUE(unemployed.empty());
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}

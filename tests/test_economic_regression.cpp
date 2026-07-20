#include "test_utils.h"
#include "../Agents/Factory.h"
#include "../Agents/Government.h"
#include "../Agents/Market.h"
#include "../Agents/Pop.h"

#include <vector>

using namespace std;

static mt19937 regression_rng(12345);

static nlohmann::json pop_config(float reproduction_rate = 0.05f) {
    return {
        {"reserve_constant", 1.0f},
        {"reproduction_rate", reproduction_rate}
    };
}

static double total_money(const vector<Pop>& pops,
                          const vector<Factory*>& factories,
                          const Government& government) {
    double total = government.reserve;
    for (const Pop& pop : pops) total += pop.money;
    for (const Factory* factory : factories) total += factory->cash;
    return total;
}

static Pop market_pop(int number, double money, float food_bid,
                      Government* government) {
    Pop pop;
    pop.number = number;
    pop.money = money;
    pop.Goods["Food"] = 0;
    pop.Goods["Cloth"] = 0;
    pop.Consumption["Money_for_Food"] = food_bid;
    pop.Consumption["Money_for_Cloth"] = 0;
    pop.gov = government;
    pop.factory = nullptr;
    return pop;
}

static Factory food_offer(int production, float price, double cash = 0) {
    Factory factory;
    factory.type = "Food";
    factory.production = production;
    factory.my_price = price;
    factory.cash = cash;
    factory.money_gained = 0;
    factory.stock_sold = 0;
    return factory;
}

TEST(pop_constructor_preserves_initial_money) {
    Government government;
    vector<Pop> pops;
    Market market(&pops);
    Pop pop(10, 321, 10, nullptr, &market, &government, pop_config());

    EXPECT_FLOAT_EQ(pop.money, 321.0f, 0.0001f);
}

TEST(paper_capacity_selects_growth_gradual_decline_and_extinction) {
    Pop fed;
    fed.number = 100;
    fed.reproduction_rate = 0.05f;
    fed.Pop_variation(regression_rng, 200);
    EXPECT_GT(fed.number, 100);

    Pop half_fed;
    half_fed.number = 100;
    half_fed.reproduction_rate = 0.05f;
    half_fed.Pop_variation(regression_rng, 50);
    EXPECT_LT(half_fed.number, 100);
    EXPECT_GT(half_fed.number, 50);

    Pop unfed;
    unfed.number = 100;
    unfed.reproduction_rate = 0.05f;
    unfed.Pop_variation(regression_rng, 0);
    EXPECT_EQ(unfed.number, 0);
}

TEST(zero_population_has_no_demand_and_remains_extinct) {
    Government government;
    Pop extinct;
    extinct.number = 0;
    extinct.money = 25;
    extinct.reproduction_rate = 0.5f;
    extinct.Goods["Food"] = 100;
    extinct.Goods["Cloth"] = 100;
    extinct.Consumption["Money_for_Food"] = 100;
    extinct.Consumption["Money_for_Cloth"] = 100;
    extinct.gov = &government;

    extinct.Food_variation();
    extinct.Pop_variation(regression_rng);
    extinct.SOL(regression_rng);

    EXPECT_EQ(extinct.number, 0);
    EXPECT_FLOAT_EQ(extinct.food_consumed(), 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(extinct.cloth_used(), 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(extinct.Goods["Food"], 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(extinct.Consumption["Money_for_Food"], 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(extinct.Consumption["Money_for_Cloth"], 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(extinct.money, 25.0f, 0.0001f);
}

TEST(zero_food_can_drive_population_to_extinction_without_a_floor) {
    Pop pop;
    pop.number = 10;
    pop.reproduction_rate = 1.0f;
    pop.Goods["Food"] = 0;

    pop.Food_variation();
    pop.Pop_variation(regression_rng);
    EXPECT_EQ(pop.number, 0);

    pop.Pop_variation(regression_rng);
    EXPECT_EQ(pop.number, 0);
    EXPECT_FLOAT_EQ(pop.Goods["Food"], 0.0f, 0.0001f);
}

TEST(payroll_is_a_conserved_transfer) {
    Government government;
    government.tax_rate = 0;

    Factory factory;
    factory.cash = 100;
    factory.money_gained = 150;
    factory.worker_count = 1;
    factory.workforce = 10;

    Pop pop;
    pop.number = 10;
    pop.money = 20;
    pop.factory = &factory;
    pop.gov = &government;

    double money_before = factory.cash + pop.money + government.reserve;
    factory.Salary();
    pop.Money();
    double money_after = factory.cash + pop.money + government.reserve;

    EXPECT_FLOAT_EQ(factory.salary, 100.0f, 0.0001f);
    EXPECT_FLOAT_EQ(money_after, money_before, 0.0001f);
}

TEST(payroll_is_distributed_by_population_share) {
    Government government;
    government.tax_rate = 0;
    Factory factory;
    factory.salary = 100;
    factory.worker_count = 2;
    factory.workforce = 100;

    Pop small;
    small.number = 25;
    small.money = 0;
    small.factory = &factory;
    small.gov = &government;

    Pop large = small;
    large.number = 75;

    small.Money();
    large.Money();

    EXPECT_FLOAT_EQ(small.money, 25.0f, 0.0001f);
    EXPECT_FLOAT_EQ(large.money, 75.0f, 0.0001f);
}

TEST(sales_payroll_and_tax_conserve_total_money_across_multiple_steps) {
    Government government;
    government.tax_rate = 0.20f;
    government.reserve = 10;

    Factory factory = food_offer(100, 2, 500);
    factory.money_gained = 120;
    factory.worker_count = 2;
    factory.workforce = 100;

    Pop small = market_pop(25, 100, 2, &government);
    small.factory = &factory;
    Pop large = market_pop(75, 200, 2, &government);
    large.factory = &factory;

    vector<Pop> pops{small, large};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);
    const double initial_money = total_money(pops, factories, government);

    factory.Salary();
    pops[0].Money();
    pops[1].Money();
    EXPECT_FLOAT_EQ(factory.salary, 120.0f, 0.0001f);
    EXPECT_FLOAT_EQ(government.reserve, 34.0f, 0.0001f);
    EXPECT_FLOAT_EQ(total_money(pops, factories, government), initial_money,
                    0.0001f);

    market.Update();
    EXPECT_FLOAT_EQ(factory.money_gained, 200.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.cash, 580.0f, 0.0001f);
    EXPECT_FLOAT_EQ(total_money(pops, factories, government), initial_money,
                    0.0001f);

    factory.Salary();
    pops[0].Money();
    pops[1].Money();
    EXPECT_FLOAT_EQ(factory.salary, 160.0f, 0.0001f);
    EXPECT_FLOAT_EQ(government.reserve, 66.0f, 0.0001f);
    EXPECT_FLOAT_EQ(total_money(pops, factories, government), initial_money,
                    0.0001f);
}

TEST(market_purchase_cannot_create_unfunded_revenue) {
    Government government;
    Factory factory;
    factory.type = "Food";
    factory.production = 10;
    factory.my_price = 10;
    factory.cash = 100;

    Pop pop;
    pop.number = 10;
    pop.money = 50;
    pop.Goods["Food"] = 0;
    pop.Consumption["Money_for_Food"] = 20;
    pop.gov = &government;

    vector<Pop> pops{pop};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);

    double money_before = pops[0].money + factory.cash;
    market.Update();
    double money_after = pops[0].money + factory.cash;

    EXPECT_FLOAT_EQ(market.Stats["Food_consumed"], 5.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].money, 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(money_after, money_before, 0.0001f);
}

TEST(market_caps_purchase_at_exact_affordability) {
    Government government;
    Factory factory = food_offer(10, 3, 7);
    vector<Pop> pops{market_pop(10, 10, 10, &government)};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);
    const double initial_money = total_money(pops, factories, government);

    market.Update();

    EXPECT_FLOAT_EQ(market.Stats["Food_consumed"], 10.0f / 3.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].Goods["Food"], 10.0f / 3.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].money, 0.0f, 0.0001f);
    EXPECT_GE(pops[0].money, -0.0001f);
    EXPECT_FLOAT_EQ(factory.cash, 17.0f, 0.0001f);
    EXPECT_FLOAT_EQ(total_money(pops, factories, government), initial_money,
                    0.0001f);
}

TEST(market_with_no_supply_or_no_affordable_offer_does_not_trade) {
    Government government;
    Factory empty = food_offer(0, 1, 20);
    Factory expensive = food_offer(10, 11, 30);
    vector<Pop> pops{market_pop(10, 100, 10, &government)};
    vector<Factory*> factories{&empty, &expensive};
    Market market(&pops, &factories);
    const double initial_money = total_money(pops, factories, government);

    market.Update();

    EXPECT_FLOAT_EQ(market.Stats["Food_consumed"], 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].Goods["Food"], 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].money, 100.0f, 0.0001f);
    EXPECT_FLOAT_EQ(empty.stock_sold, 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(expensive.stock_sold, 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(total_money(pops, factories, government), initial_money,
                    0.0001f);
}

TEST(market_never_sells_more_than_supply_or_demand) {
    Government government;

    Factory scarce = food_offer(4, 2);
    vector<Pop> scarce_pops{market_pop(10, 100, 2, &government)};
    vector<Factory*> scarce_factories{&scarce};
    Market scarce_market(&scarce_pops, &scarce_factories);
    scarce_market.Update();
    EXPECT_FLOAT_EQ(scarce_market.Stats["Food_consumed"], 4.0f, 0.0001f);
    EXPECT_FLOAT_EQ(scarce.stock_sold, 4.0f, 0.0001f);

    Factory abundant = food_offer(20, 2);
    vector<Pop> abundant_pops{market_pop(10, 100, 2, &government)};
    vector<Factory*> abundant_factories{&abundant};
    Market abundant_market(&abundant_pops, &abundant_factories);
    abundant_market.Update();
    EXPECT_FLOAT_EQ(abundant_market.Stats["Food_consumed"], 10.0f, 0.0001f);
    EXPECT_FLOAT_EQ(abundant.stock_sold, 10.0f, 0.0001f);
}

TEST(market_uses_cheapest_supply_first_and_reports_weighted_price) {
    Government government;
    Factory expensive = food_offer(10, 5);
    Factory cheap = food_offer(4, 2);
    vector<Pop> pops{market_pop(10, 100, 5, &government)};
    vector<Factory*> factories{&expensive, &cheap};
    Market market(&pops, &factories);

    market.Update();

    EXPECT_FLOAT_EQ(cheap.stock_sold, 4.0f, 0.0001f);
    EXPECT_FLOAT_EQ(expensive.stock_sold, 6.0f, 0.0001f);
    EXPECT_FLOAT_EQ(market.Stats["Food_consumed"], 10.0f, 0.0001f);
    EXPECT_FLOAT_EQ(market.Stats["Food_value"], 3.8f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].money, 62.0f, 0.0001f);
}

TEST(zero_price_goods_do_not_change_the_money_supply) {
    Government government;
    Factory factory = food_offer(7, 0, 13);
    vector<Pop> pops{market_pop(10, 0, 1, &government)};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);
    const double initial_money = total_money(pops, factories, government);

    market.Update();

    EXPECT_FLOAT_EQ(market.Stats["Food_consumed"], 7.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].money, 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.cash, 13.0f, 0.0001f);
    EXPECT_FLOAT_EQ(total_money(pops, factories, government), initial_money,
                    0.0001f);
}

TEST(factory_without_workers_does_not_run_payroll) {
    Factory factory;
    factory.cash = 100;
    factory.salary = 25;
    factory.money_gained = 50;
    factory.worker_count = 0;
    factory.workforce = 0;

    factory.Salary();

    EXPECT_FLOAT_EQ(factory.cash, 100.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.salary, 25.0f, 0.0001f);
}

TEST(factory_without_workforce_produces_nothing_and_mints_no_revenue) {
    Government government;
    vector<Pop> pops;
    vector<Factory*> factories;
    Market market(&pops, &factories);

    Factory factory;
    factory.type = "Food";
    factory.markets = &market;
    factory.workforce = 0;
    factory.production = 99;
    factory.stock_sold = 12;
    factory.cash = 41;
    factory.money_gained = 17;

    factory.Production();

    EXPECT_EQ(factory.production, 0);
    EXPECT_FLOAT_EQ(factory.stock_sold, 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.cash, 41.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.money_gained, 17.0f, 0.0001f);
    EXPECT_FLOAT_EQ(market.Stats["Food_produced"], 0.0f, 0.0001f);
}

TEST(production_changes_goods_output_but_not_any_money_balance) {
    Government government;
    vector<Pop> pops;
    vector<Factory*> factories;
    Market market(&pops, &factories);

    Factory factory;
    factory.type = "Food";
    factory.markets = &market;
    factory.workforce = 50;
    factory.Efficiency_parameter = 20;
    factory.distribution_parameter = 0.35f;
    factory.sigma_sostitution = -0.25f;
    factory.investment = 10;
    factory.cash = 123;
    factory.money_gained = 45;

    factory.Production();

    EXPECT_GT(factory.production, 0);
    EXPECT_FLOAT_EQ(market.Stats["Food_produced"], factory.production, 0.0001f);
    EXPECT_FLOAT_EQ(factory.cash, 123.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.money_gained, 45.0f, 0.0001f);
}

TEST(empty_economy_updates_without_creating_money_goods_or_people) {
    Government government;
    vector<Pop> pops;
    vector<Factory*> factories;
    Market market(&pops, &factories);
    government = Government(&factories, &pops, 0.25f, &market);

    market.Update();
    government.Update();

    EXPECT_FLOAT_EQ(government.reserve, 0.0f, 0.0001f);
    EXPECT_EQ(government.n_tot, 0);
    EXPECT_EQ(government.n_food, 0);
    EXPECT_FLOAT_EQ(government.gdp, 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(market.Stats["Food_consumed"], 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(market.Stats["Cloth_consumed"], 0.0f, 0.0001f);
}

TEST(deterministic_multi_period_population_and_wages_reverse_direction) {
    Government government;
    government.tax_rate = 0.25f;

    Factory factory = food_offer(0, 1, 1000);
    factory.type = "Cloth";
    factory.worker_count = 1;
    factory.workforce = 100;
    factory.money_gained = 20;

    Pop pop = market_pop(100, 1000, 1, &government);
    pop.factory = &factory;
    pop.reproduction_rate = 0.05f;
    pop.Goods["Food"] = 0;

    Pop buyer = market_pop(1200, 1000, 0, &government);
    buyer.Consumption["Money_for_Cloth"] = 1;
    vector<Pop> pops{pop, buyer};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);
    const vector<int> offered_cloth{20, 80, 80, 0, 0};
    const vector<float> capacity_ratio{1.5f, 0.5f, 1.5f, 1.5f, 0.5f};
    vector<int> populations;
    vector<double> wages;
    const double initial_money = total_money(pops, factories, government);

    for (size_t period = 0; period < offered_cloth.size(); ++period) {
        factory.worker_count = 1;
        factory.workforce = pops[0].number;
        factory.Salary();
        pops[0].Money();
        wages.push_back(factory.salary);

        float carrying_capacity = pops[0].number * capacity_ratio[period];
        pops[0].Pop_variation(regression_rng, carrying_capacity);
        populations.push_back(pops[0].number);

        pops[1].Goods["Cloth"] = 0;
        factory.production = offered_cloth[period];
        factory.stock_sold = 0;
        market.Stats["Cloth_consumed"] = 0;
        market.Update();
        EXPECT_FLOAT_EQ(total_money(pops, factories, government), initial_money,
                        0.0002f);
    }

    EXPECT_TRUE(populations[1] < populations[0]);
    EXPECT_TRUE(populations[2] > populations[1]);
    EXPECT_TRUE(populations[4] < populations[3]);
    EXPECT_TRUE(wages[2] > wages[1]);
    EXPECT_TRUE(wages[4] < wages[3]);
    EXPECT_FLOAT_EQ(wages[0], 20.0f, 0.0001f);
    EXPECT_FLOAT_EQ(wages[2], 40.0f, 0.0001f);
    EXPECT_FLOAT_EQ(wages[3], 50.0f, 0.0001f);
    EXPECT_FLOAT_EQ(wages[4], 40.0f, 0.0001f);
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}

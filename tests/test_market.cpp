#include "test_utils.h"
#include "../Agents/Factory.h"
#include "../Agents/Government.h"
#include "../Agents/Market.h"
#include "../Agents/Pop.h"
#include "../lib/Algorithms.h"

#include <vector>

using namespace std;

static Factory offer(const string& type, int stock, float price, double cash = 0) {
    Factory factory;
    factory.type = type;
    factory.production = stock;
    factory.my_price = price;
    factory.cash = cash;
    factory.money_gained = 0;
    factory.stock_sold = 0;
    return factory;
}

static Pop bidder(int number, double money, float food_bid, float cloth_bid = 0) {
    Pop pop;
    pop.number = number;
    pop.money = money;
    pop.Goods["Food"] = 0;
    pop.Goods["Cloth"] = 0;
    pop.Consumption["Money_for_Food"] = food_bid;
    pop.Consumption["Money_for_Cloth"] = cloth_bid;
    return pop;
}

static double monetary_total(const vector<Pop>& pops,
                             const vector<Factory*>& factories) {
    double total = 0;
    for (const Pop& pop : pops) total += pop.money;
    for (const Factory* factory : factories) total += factory->cash;
    return total;
}

TEST(constructor_initializes_both_good_statistics) {
    vector<Pop> pops;
    vector<Factory*> factories;
    Market market(&pops, &factories);

    EXPECT_EQ(market.Pops, &pops);
    EXPECT_EQ(market.Factories, &factories);
    EXPECT_FLOAT_EQ(market.Stats["Food_value"], 100.0f, 0.0001f);
    EXPECT_FLOAT_EQ(market.Stats["Cloth_value"], 100.0f, 0.0001f);
    EXPECT_FLOAT_EQ(market.Stats["Food_consumed"], 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(market.Stats["Cloth_consumed"], 0.0f, 0.0001f);
}

TEST(single_argument_market_has_no_factory_book_and_update_is_a_noop) {
    vector<Pop> pops{bidder(10, 100, 10)};
    Market market(&pops);
    market.Update();
    EXPECT_EQ(market.Factories, nullptr);
    EXPECT_FLOAT_EQ(pops[0].money, 100.0, 0.0001);
}

TEST(food_market_fills_demand_when_supply_is_abundant) {
    Factory factory = offer("Food", 20, 2, 10);
    vector<Pop> pops{bidder(10, 100, 2)};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);

    market.ClearMarket("Food");

    EXPECT_FLOAT_EQ(pops[0].Goods["Food"], 10.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.stock_sold, 10.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].money, 80.0, 0.0001);
    EXPECT_FLOAT_EQ(factory.cash, 30.0, 0.0001);
}

TEST(food_market_never_sells_more_than_available_supply) {
    Factory factory = offer("Food", 4, 2);
    vector<Pop> pops{bidder(10, 100, 2)};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);

    market.ClearMarket("Food");
    EXPECT_FLOAT_EQ(market.Stats["Food_consumed"], 4.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.stock_sold, 4.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].Goods["Food"], 4.0f, 0.0001f);
}

TEST(no_supply_leaves_money_goods_and_price_unchanged) {
    Factory factory = offer("Food", 0, 2, 10);
    vector<Pop> pops{bidder(10, 100, 2)};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);

    market.ClearMarket("Food");
    EXPECT_FLOAT_EQ(pops[0].money, 100.0, 0.0001);
    EXPECT_FLOAT_EQ(pops[0].Goods["Food"], 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.cash, 10.0, 0.0001);
    EXPECT_FLOAT_EQ(market.Stats["Food_value"], 100.0f, 0.0001f);
}

TEST(bid_below_offer_price_does_not_trade) {
    Factory factory = offer("Food", 10, 6);
    vector<Pop> pops{bidder(10, 100, 5)};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);

    market.ClearMarket("Food");
    EXPECT_FLOAT_EQ(market.Stats["Food_consumed"], 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(factory.stock_sold, 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].money, 100.0, 0.0001);
}

TEST(purchase_is_capped_by_buyer_affordability) {
    Factory factory = offer("Food", 10, 3, 7);
    vector<Pop> pops{bidder(10, 10, 3)};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);
    double before = monetary_total(pops, factories);

    market.ClearMarket("Food");

    EXPECT_FLOAT_EQ(pops[0].Goods["Food"], 10.0f / 3.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].money, 0.0, 0.0001);
    EXPECT_GE(pops[0].money, -0.0001);
    EXPECT_FLOAT_EQ(monetary_total(pops, factories), before, 0.0001);
}

TEST(highest_bidder_receives_scarce_supply_first) {
    Factory factory = offer("Food", 5, 4);
    vector<Pop> pops{bidder(5, 100, 4), bidder(5, 100, 8)};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);

    market.ClearMarket("Food");

    EXPECT_FLOAT_EQ(pops[1].Goods["Food"], 5.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].Goods["Food"], 0.0f, 0.0001f);
}

TEST(cheapest_factory_is_used_before_expensive_factory) {
    Factory expensive = offer("Food", 10, 5);
    Factory cheap = offer("Food", 4, 2);
    vector<Pop> pops{bidder(10, 100, 5)};
    vector<Factory*> factories{&expensive, &cheap};
    Market market(&pops, &factories);

    market.ClearMarket("Food");

    EXPECT_FLOAT_EQ(cheap.stock_sold, 4.0f, 0.0001f);
    EXPECT_FLOAT_EQ(expensive.stock_sold, 6.0f, 0.0001f);
    EXPECT_FLOAT_EQ(market.Stats["Food_value"], 3.8f, 0.0001f);
}

TEST(market_sale_is_an_exact_transfer_between_buyer_and_factory) {
    Factory factory = offer("Food", 10, 7, 30);
    vector<Pop> pops{bidder(10, 100, 7)};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);
    double before = monetary_total(pops, factories);

    market.ClearMarket("Food");

    EXPECT_FLOAT_EQ(factory.money_gained, 70.0, 0.0001);
    EXPECT_FLOAT_EQ(monetary_total(pops, factories), before, 0.0001);
}

TEST(cloth_market_uses_cloth_deficit_not_food_demand) {
    Factory factory = offer("Cloth", 20, 3);
    Pop pop = bidder(120, 100, 0, 3);
    pop.Goods["Cloth"] = 4;
    vector<Pop> pops{pop};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);

    market.ClearMarket("Cloth");

    EXPECT_FLOAT_EQ(market.Stats["Cloth_consumed"], 6.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].Goods["Cloth"], 10.0f, 0.0001f);
    EXPECT_FLOAT_EQ(market.Stats["Food_consumed"], 0.0f, 0.0001f);
}

TEST(satisfied_cloth_demand_does_not_trade) {
    Factory factory = offer("Cloth", 20, 3);
    Pop pop = bidder(120, 100, 0, 3);
    pop.Goods["Cloth"] = 10;
    vector<Pop> pops{pop};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);

    market.ClearMarket("Cloth");
    EXPECT_FLOAT_EQ(factory.stock_sold, 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].money, 100.0, 0.0001);
}

TEST(update_clears_food_and_cloth_and_resets_period_revenue) {
    Factory food = offer("Food", 10, 2);
    Factory cloth = offer("Cloth", 10, 4);
    food.money_gained = 999;
    cloth.money_gained = 999;
    vector<Pop> pops{bidder(12, 200, 4, 4)};
    vector<Factory*> factories{&food, &cloth};
    Market market(&pops, &factories);

    market.Update();

    EXPECT_FLOAT_EQ(food.money_gained, 20.0, 0.0001);
    EXPECT_FLOAT_EQ(cloth.money_gained, 4.0, 0.0001);
    EXPECT_FLOAT_EQ(market.Stats["Food_consumed"], 10.0f, 0.0001f);
    EXPECT_FLOAT_EQ(market.Stats["Cloth_consumed"], 1.0f, 0.0001f);
}

TEST(transaction_price_and_inflation_are_volume_weighted) {
    Factory cheap = offer("Food", 2, 5);
    Factory costly = offer("Food", 2, 15);
    vector<Pop> pops{bidder(4, 100, 15)};
    vector<Factory*> factories{&costly, &cheap};
    Market market(&pops, &factories);
    market.Stats["Food_value"] = 20;

    market.ClearMarket("Food");

    EXPECT_FLOAT_EQ(market.Stats["Food_value"], 10.0f, 0.0001f);
    EXPECT_FLOAT_EQ(market.Stats["Food_Inflation"], -0.5f, 0.0001f);
}

TEST(consumption_statistics_accumulate_until_the_caller_resets_them) {
    Factory factory = offer("Food", 3, 1);
    vector<Pop> pops{bidder(3, 100, 1)};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);

    market.ClearMarket("Food");
    factory.production = 3;
    factory.stock_sold = 0;
    pops[0].Goods["Food"] = 0;
    market.ClearMarket("Food");

    EXPECT_FLOAT_EQ(market.Stats["Food_consumed"], 6.0f, 0.0001f);
}

TEST(zero_price_goods_transfer_without_changing_money) {
    Factory factory = offer("Food", 7, 0, 10);
    vector<Pop> pops{bidder(10, 0, 1)};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);

    market.ClearMarket("Food");

    EXPECT_FLOAT_EQ(factory.stock_sold, 7.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].money, 0.0, 0.0001);
    EXPECT_FLOAT_EQ(factory.cash, 10.0, 0.0001);
}

TEST(extinct_population_has_zero_market_demand) {
    Factory factory = offer("Food", 10, 1);
    vector<Pop> pops{bidder(0, 100, 10, 10)};
    vector<Factory*> factories{&factory};
    Market market(&pops, &factories);

    market.Update();

    EXPECT_FLOAT_EQ(factory.stock_sold, 0.0f, 0.0001f);
    EXPECT_FLOAT_EQ(pops[0].money, 100.0, 0.0001);
}

TEST(sort_bids_orders_requested_column_descending) {
    vector<vector<float>> bids{{0, 2, 9}, {1, 8, 3}, {2, 5, 6}};
    SortBids(bids, 1);
    EXPECT_FLOAT_EQ(bids[0][1], 8.0f, 0.0001f);
    EXPECT_FLOAT_EQ(bids[1][1], 5.0f, 0.0001f);
    EXPECT_FLOAT_EQ(bids[2][1], 2.0f, 0.0001f);
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}

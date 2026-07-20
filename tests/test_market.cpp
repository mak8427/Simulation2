#include "test_utils.h"
#include "../Agents/Market.h"
#include "../Agents/Pop.h"
#include "../Agents/Factory.h"
#include "../Agents/Government.h"
#include "../lib/Algorithms.h"

#include <cmath>
#include <vector>
#include <string>

using namespace std;

static Factory make_factory(float salary = 1000.0f) {
    Factory f;
    f.salary = salary;
    return f;
}

static Government make_gov(float tax_rate = 0.0f) {
    Government g;
    g.tax_rate = tax_rate;
    g.reserve = 0.0f;
    return g;
}

static Pop make_pop(int number, float money, float food_goods, float cloth_goods,
                    float money_for_food, float money_for_cloth,
                    Factory* factory, Government* gov) {
    Pop p;
    p.number = number;
    p.money = money;
    p.Goods["Food"] = food_goods;
    p.Goods["Cloth"] = cloth_goods;
    p.Consumption["Money_for_Food"] = money_for_food;
    p.Consumption["Money_for_Cloth"] = money_for_cloth;
    p.factory = factory;
    p.gov = gov;
    p.markets = nullptr;
    return p;
}

// ==================== Market Constructor Tests ====================

TEST(Market_Constructor_Defaults) {
    vector<Pop> pops;
    Market m(&pops);

    EXPECT_EQ(m.Pops, &pops);
    EXPECT_EQ(m.Goods.size(), static_cast<size_t>(2));
    EXPECT_EQ(m.Goods[0], "Food");
    EXPECT_EQ(m.Goods[1], "Cloth");

    EXPECT_EQ(m.Stats["Food_value"], 100);
    EXPECT_EQ(m.Stats["Food_Inflation"], 0);
    EXPECT_EQ(m.Stats["Food_produced"], 0);
    EXPECT_EQ(m.Stats["Food_consumed"], 0);
    EXPECT_EQ(m.Stats["Cloth_value"], 100);
    EXPECT_EQ(m.Stats["Cloth_Inflation"], 0);
    EXPECT_EQ(m.Stats["Cloth_produced"], 0);
    EXPECT_EQ(m.Stats["Cloth_consumed"], 0);
}

TEST(Market_Constructor_PopsSet) {
    vector<Pop> pops;
    Pop p;
    p.number = 42;
    pops.push_back(p);
    Market m(&pops);

    EXPECT_EQ(m.Pops, &pops);
    EXPECT_EQ(m.Pops->size(), static_cast<size_t>(1));
    EXPECT_EQ(m.Pops->at(0).number, 42);
}

// ==================== Sender() - Food Market Clearing ====================

TEST(Sender_Food_SupplyExceedsDemand) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 1000, 0, 0, 50, 0, &f, &g);
    Pop b = make_pop(5, 1000, 0, 0, 30, 0, &f, &g);
    Pop c = make_pop(3, 1000, 0, 0, 20, 0, &f, &g);
    pops.push_back(a);
    pops.push_back(b);
    pops.push_back(c);

    Market m(&pops);
    m.Stats["Food_produced"] = 30;

    m.Sender();

    EXPECT_EQ(m.Stats["Food_value"], 20);
    EXPECT_EQ(m.Stats["Food_consumed"], 18);
    EXPECT_EQ(m.Pops->at(0).money, 1000 - 50 * 10);   // 500
    EXPECT_EQ(m.Pops->at(1).money, 1000 - 30 * 5);    // 850
    EXPECT_EQ(m.Pops->at(2).money, 1000 - 20 * 3);    // 940
    EXPECT_EQ(m.Pops->at(0).Goods["Food"], 10);
    EXPECT_EQ(m.Pops->at(1).Goods["Food"], 5);
    EXPECT_EQ(m.Pops->at(2).Goods["Food"], 3);
}

TEST(Sender_Food_SupplyLessThanDemand) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 1000, 0, 0, 50, 0, &f, &g);
    Pop b = make_pop(5, 1000, 0, 0, 30, 0, &f, &g);
    Pop c = make_pop(3, 1000, 0, 0, 20, 0, &f, &g);
    pops.push_back(a);
    pops.push_back(b);
    pops.push_back(c);

    Market m(&pops);
    m.Stats["Food_produced"] = 12;

    m.Sender();

    EXPECT_EQ(m.Stats["Food_value"], 30);
    EXPECT_EQ(m.Stats["Food_consumed"], 12);
    EXPECT_EQ(m.Pops->at(0).Goods["Food"], 10);
    EXPECT_EQ(m.Pops->at(1).Goods["Food"], 2);
    EXPECT_EQ(m.Pops->at(2).Goods["Food"], 0);
    EXPECT_EQ(m.Pops->at(0).money, 1000 - 50 * 10);
    EXPECT_EQ(m.Pops->at(1).money, 1000 - 30 * 2);
    EXPECT_EQ(m.Pops->at(2).money, 1000);
}

TEST(Sender_Food_NoSupply) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 1000, 0, 0, 50, 0, &f, &g);
    pops.push_back(a);

    Market m(&pops);
    m.Stats["Food_produced"] = 0;

    m.Sender();

    EXPECT_EQ(m.Stats["Food_consumed"], 0);
    EXPECT_EQ(m.Pops->at(0).Goods["Food"], 0);
    EXPECT_EQ(m.Pops->at(0).money, 1000);
}

TEST(Sender_Food_ZeroMoneyForFood) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 1000, 0, 0, 0, 0, &f, &g);
    Pop b = make_pop(5, 1000, 0, 0, 0, 0, &f, &g);
    pops.push_back(a);
    pops.push_back(b);

    Market m(&pops);
    m.Stats["Food_produced"] = 20;

    m.Sender();

    EXPECT_EQ(m.Stats["Food_consumed"], 15);
    EXPECT_EQ(m.Stats["Food_value"], 0);
    EXPECT_EQ(m.Pops->at(0).money, 1000);
    EXPECT_EQ(m.Pops->at(1).money, 1000);
    EXPECT_EQ(m.Pops->at(0).Goods["Food"], 10);
    EXPECT_EQ(m.Pops->at(1).Goods["Food"], 5);
}

TEST(Sender_Food_ExactSupplyEqualsDemand) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 1000, 0, 0, 50, 0, &f, &g);
    Pop b = make_pop(5, 1000, 0, 0, 30, 0, &f, &g);
    Pop c = make_pop(3, 1000, 0, 0, 20, 0, &f, &g);
    pops.push_back(a);
    pops.push_back(b);
    pops.push_back(c);

    Market m(&pops);
    m.Stats["Food_produced"] = 18;

    m.Sender();

    EXPECT_EQ(m.Stats["Food_value"], 20);
    EXPECT_EQ(m.Stats["Food_consumed"], 18);
    EXPECT_EQ(m.Pops->at(0).Goods["Food"], 10);
    EXPECT_EQ(m.Pops->at(1).Goods["Food"], 5);
    EXPECT_EQ(m.Pops->at(2).Goods["Food"], 3);
    EXPECT_EQ(m.Pops->at(0).money, 1000 - 50 * 10);
    EXPECT_EQ(m.Pops->at(1).money, 1000 - 30 * 5);
    EXPECT_EQ(m.Pops->at(2).money, 1000 - 20 * 3);
}

TEST(Sender_Food_SinglePopMarginalFill) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 1000, 0, 0, 50, 0, &f, &g);
    pops.push_back(a);

    Market m(&pops);
    m.Stats["Food_produced"] = 3;

    m.Sender();

    EXPECT_EQ(m.Stats["Food_consumed"], 3);
    EXPECT_EQ(m.Stats["Food_value"], 50);
    EXPECT_EQ(m.Pops->at(0).Goods["Food"], 3);
    EXPECT_EQ(m.Pops->at(0).money, 1000 - 50 * 3);
}

TEST(Sender_Food_FoodConsumedAccumulates) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 1000, 0, 0, 50, 0, &f, &g);
    pops.push_back(a);

    Market m(&pops);
    m.Stats["Food_produced"] = 5;

    m.Sender();
    EXPECT_EQ(m.Stats["Food_consumed"], 5);

    m.Sender();
    EXPECT_EQ(m.Stats["Food_consumed"], 10);
}

TEST(Sender_Food_InflationCalculation) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 2000, 0, 0, 50, 0, &f, &g);
    Pop b = make_pop(5, 2000, 0, 0, 30, 0, &f, &g);
    pops.push_back(a);
    pops.push_back(b);

    Market m(&pops);
    m.Stats["Food_produced"] = 20;

    float old_value = m.Stats["Food_value"];
    m.Sender();

    float new_value = m.Stats["Food_value"];
    float expected_inflation = (new_value / old_value) - 1;
    EXPECT_EQ(m.Stats["Food_Inflation"], expected_inflation);
}

TEST(Sender_Food_MultipleBiddersTied) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(5, 1000, 0, 0, 40, 0, &f, &g);
    Pop b = make_pop(5, 1000, 0, 0, 40, 0, &f, &g);
    pops.push_back(a);
    pops.push_back(b);

    Market m(&pops);
    m.Stats["Food_produced"] = 7;

    m.Sender();

    EXPECT_EQ(m.Stats["Food_consumed"], 7);
    EXPECT_EQ(m.Stats["Food_value"], 40);
}

// ==================== Sender_cloth() - Cloth Market Clearing ====================

TEST(Sender_Cloth_SupplyExceedsDemand) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(24, 1000, 0, 0, 0, 50, &f, &g);
    Pop b = make_pop(12, 1000, 0, 0, 0, 30, &f, &g);
    Pop c = make_pop(12, 1000, 0, 0, 0, 20, &f, &g);
    pops.push_back(a);
    pops.push_back(b);
    pops.push_back(c);

    Market m(&pops);
    m.Stats["Cloth_produced"] = 10;

    m.Sender_cloth();

    EXPECT_EQ(m.Stats["Cloth_value"], 20);
    EXPECT_TRUE(m.Stats["Cloth_consumed"] > 0);
    EXPECT_EQ(m.Pops->at(0).Goods["Cloth"], 2);
    EXPECT_EQ(m.Pops->at(1).Goods["Cloth"], 1);
    EXPECT_EQ(m.Pops->at(2).Goods["Cloth"], 1);
    EXPECT_EQ(m.Pops->at(0).money, 1000 - 50 * 2);
    EXPECT_EQ(m.Pops->at(1).money, 1000 - 30 * 1);
    EXPECT_EQ(m.Pops->at(2).money, 1000 - 20 * 1);
}

TEST(Sender_Cloth_SupplyLessThanDemand) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(24, 1000, 0, 0, 0, 50, &f, &g);
    Pop b = make_pop(12, 1000, 0, 0, 0, 30, &f, &g);
    Pop c = make_pop(12, 1000, 0, 0, 0, 20, &f, &g);
    pops.push_back(a);
    pops.push_back(b);
    pops.push_back(c);

    Market m(&pops);
    m.Stats["Cloth_produced"] = 2;

    m.Sender_cloth();

    EXPECT_EQ(m.Stats["Cloth_value"], 30);
    EXPECT_EQ(m.Stats["Cloth_consumed"], 2);
    EXPECT_EQ(m.Pops->at(0).Goods["Cloth"], 2);
    EXPECT_EQ(m.Pops->at(1).Goods["Cloth"], 0);
    EXPECT_EQ(m.Pops->at(2).Goods["Cloth"], 0);
    EXPECT_EQ(m.Pops->at(0).money, 1000 - 50 * 2);
    EXPECT_EQ(m.Pops->at(1).money, 1000);
    EXPECT_EQ(m.Pops->at(2).money, 1000);
}

TEST(Sender_Cloth_NoSupply) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(24, 1000, 0, 0, 0, 50, &f, &g);
    pops.push_back(a);

    Market m(&pops);
    m.Stats["Cloth_produced"] = 0;

    m.Sender_cloth();

    EXPECT_EQ(m.Stats["Cloth_consumed"], 0);
    EXPECT_EQ(m.Pops->at(0).Goods["Cloth"], 0);
    EXPECT_EQ(m.Pops->at(0).money, 1000);
}

TEST(Sender_Cloth_ZeroMoneyForCloth) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(24, 1000, 0, 0, 0, 0, &f, &g);
    Pop b = make_pop(12, 1000, 0, 0, 0, 0, &f, &g);
    pops.push_back(a);
    pops.push_back(b);

    Market m(&pops);
    m.Stats["Cloth_produced"] = 10;

    m.Sender_cloth();

    EXPECT_EQ(m.Stats["Cloth_consumed"], 3);
    EXPECT_EQ(m.Stats["Cloth_value"], 0);
    EXPECT_EQ(m.Pops->at(0).money, 1000);
    EXPECT_EQ(m.Pops->at(1).money, 1000);
    EXPECT_EQ(m.Pops->at(0).Goods["Cloth"], 2);
    EXPECT_EQ(m.Pops->at(1).Goods["Cloth"], 1);
}

TEST(Sender_Cloth_AlreadySatisfiedDemandZero) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(12, 1000, 0, 2, 0, 30, &f, &g);
    pops.push_back(a);

    Market m(&pops);
    m.Stats["Cloth_produced"] = 5;

    m.Sender_cloth();

    EXPECT_EQ(m.Stats["Cloth_consumed"], 0);
    EXPECT_EQ(m.Pops->at(0).Goods["Cloth"], 2);
    EXPECT_EQ(m.Pops->at(0).money, 1000);
}

TEST(Sender_Cloth_PartialSatisfaction) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(36, 1000, 0, 1, 0, 50, &f, &g);
    pops.push_back(a);

    Market m(&pops);
    m.Stats["Cloth_produced"] = 10;

    m.Sender_cloth();

    EXPECT_EQ(m.Stats["Cloth_value"], 50);
    EXPECT_TRUE(m.Stats["Cloth_consumed"] > 0);
}

// ==================== value_change() Tests ====================

TEST(ValueChange_Food_ProducedExceedsConsumed) {
    vector<Pop> pops;
    Market m(&pops);
    m.Stats["Food_value"] = 100;
    m.Stats["Food_produced"] = 20;
    m.Stats["Food_consumed"] = 10;

    float old_value = m.Stats["Food_value"];
    m.value_change();

    float rect = 10.0f / 20.0f - 1.0f;
    float expected = old_value + rect * log(old_value);
    EXPECT_FLOAT_EQ(m.Stats["Food_value"], expected, 0.001f);
}

TEST(ValueChange_Cloth_ProducedExceedsConsumed) {
    vector<Pop> pops;
    Market m(&pops);
    m.Stats["Cloth_value"] = 50;
    m.Stats["Cloth_produced"] = 30;
    m.Stats["Cloth_consumed"] = 15;

    float old_value = m.Stats["Cloth_value"];
    m.value_change();

    float rect = 15.0f / 30.0f - 1.0f;
    float expected = old_value + rect * log(old_value);
    EXPECT_FLOAT_EQ(m.Stats["Cloth_value"], expected, 0.001f);
}

TEST(ValueChange_Food_ProducedLessThanConsumed) {
    vector<Pop> pops;
    Market m(&pops);
    m.Stats["Food_value"] = 100;
    m.Stats["Food_produced"] = 10;
    m.Stats["Food_consumed"] = 20;

    float old_value = m.Stats["Food_value"];
    m.value_change();

    float rect = (1.0f - 1.0f / (10.0f / 20.0f)) / 5.0f;
    float expected = old_value * (1.0f - rect);
    EXPECT_FLOAT_EQ(m.Stats["Food_value"], expected, 0.001f);
}

TEST(ValueChange_Cloth_ProducedLessThanConsumed) {
    vector<Pop> pops;
    Market m(&pops);
    m.Stats["Cloth_value"] = 200;
    m.Stats["Cloth_produced"] = 5;
    m.Stats["Cloth_consumed"] = 25;

    float old_value = m.Stats["Cloth_value"];
    m.value_change();

    float rect = (1.0f - 1.0f / (5.0f / 25.0f)) / 5.0f;
    float expected = old_value * (1.0f - rect);
    EXPECT_FLOAT_EQ(m.Stats["Cloth_value"], expected, 0.001f);
}

TEST(ValueChange_Food_ProducedEqualsConsumed) {
    vector<Pop> pops;
    Market m(&pops);
    m.Stats["Food_value"] = 100;
    m.Stats["Food_produced"] = 15;
    m.Stats["Food_consumed"] = 15;
    m.Stats["Cloth_value"] = 100;
    m.Stats["Cloth_produced"] = 0;
    m.Stats["Cloth_consumed"] = 0;

    m.value_change();

    EXPECT_EQ(m.Stats["Food_value"], 100);
}

TEST(ValueChange_Cloth_ProducedEqualsConsumed) {
    vector<Pop> pops;
    Market m(&pops);
    m.Stats["Food_value"] = 100;
    m.Stats["Food_produced"] = 0;
    m.Stats["Food_consumed"] = 0;
    m.Stats["Cloth_value"] = 200;
    m.Stats["Cloth_produced"] = 5;
    m.Stats["Cloth_consumed"] = 5;

    m.value_change();

    EXPECT_EQ(m.Stats["Cloth_value"], 200);
}

TEST(ValueChange_ZeroProducedZeroConsumed) {
    vector<Pop> pops;
    Market m(&pops);
    m.Stats["Food_value"] = 100;
    m.Stats["Food_produced"] = 0;
    m.Stats["Food_consumed"] = 0;
    m.Stats["Cloth_value"] = 200;
    m.Stats["Cloth_produced"] = 0;
    m.Stats["Cloth_consumed"] = 0;

    m.value_change();

    EXPECT_TRUE(!std::isnan(m.Stats["Food_value"]));
    EXPECT_TRUE(!std::isnan(m.Stats["Cloth_value"]));
}

TEST(ValueChange_LargeRatio) {
    vector<Pop> pops;
    Market m(&pops);
    m.Stats["Food_value"] = 100;
    m.Stats["Food_produced"] = 1000;
    m.Stats["Food_consumed"] = 1;

    float old_value = m.Stats["Food_value"];
    m.value_change();

    float rect = 1.0f / 1000.0f - 1.0f;
    float expected = old_value + rect * log(old_value);
    EXPECT_FLOAT_EQ(m.Stats["Food_value"], expected, 0.001f);
    EXPECT_GT(m.Stats["Food_value"], 0);
}

// ==================== Sort_Vector Tests ====================

TEST(SortVector_DescendingOrder) {
    vector<vector<float>> matrix = {
        {0, 10, 100},
        {1, 50, 200},
        {2, 30, 300},
        {3, 60, 400},
        {4, 20, 500}
    };

    auto result = Sort_Vector(matrix, 1, matrix.size(), false);

    EXPECT_EQ(result.size(), static_cast<size_t>(5));
    EXPECT_EQ(result[0][1], 60);
    EXPECT_EQ(result[1][1], 50);
    EXPECT_EQ(result[2][1], 30);
    EXPECT_EQ(result[3][1], 20);
    EXPECT_EQ(result[4][1], 10);
}

TEST(SortVector_PreservesDataIntegrity) {
    vector<vector<float>> matrix = {
        {0, 50, 100},
        {1, 10, 200},
        {2, 30, 300}
    };

    auto result = Sort_Vector(matrix, 1, matrix.size(), false);

    EXPECT_EQ(result[0][0], 0);
    EXPECT_EQ(result[0][2], 100);
    EXPECT_EQ(result[1][0], 2);
    EXPECT_EQ(result[1][2], 300);
    EXPECT_EQ(result[2][0], 1);
    EXPECT_EQ(result[2][2], 200);
}

TEST(SortVector_EmptyVector) {
    vector<vector<float>> matrix;

    auto result = Sort_Vector(matrix, 0, 0, false);

    EXPECT_TRUE(result.empty());
}

TEST(SortVector_SingleElement) {
    vector<vector<float>> matrix = {{7, 42, 99}};

    auto result = Sort_Vector(matrix, 1, matrix.size(), false);

    EXPECT_EQ(result.size(), static_cast<size_t>(1));
    EXPECT_EQ(result[0][0], 7);
    EXPECT_EQ(result[0][1], 42);
    EXPECT_EQ(result[0][2], 99);
}

TEST(SortVector_AlreadySorted) {
    vector<vector<float>> matrix = {
        {0, 100, 0},
        {1, 80, 0},
        {2, 50, 0},
        {3, 20, 0}
    };

    auto result = Sort_Vector(matrix, 1, matrix.size(), false);

    EXPECT_EQ(result.size(), matrix.size());
    EXPECT_EQ(result[0][1], 100);
    EXPECT_EQ(result[1][1], 80);
    EXPECT_EQ(result[2][1], 50);
    EXPECT_EQ(result[3][1], 20);
}

TEST(SortVector_ReverseSorted) {
    vector<vector<float>> matrix = {
        {0, 5, 10},
        {1, 15, 20},
        {2, 25, 30},
        {3, 35, 40}
    };

    auto result = Sort_Vector(matrix, 1, matrix.size(), false);

    EXPECT_EQ(result[0][1], 35);
    EXPECT_EQ(result[1][1], 25);
    EXPECT_EQ(result[2][1], 15);
    EXPECT_EQ(result[3][1], 5);
}

TEST(SortVector_SortByColumn0) {
    vector<vector<float>> matrix = {
        {3, 10, 100},
        {1, 20, 200},
        {4, 30, 300},
        {2, 40, 400}
    };

    auto result = Sort_Vector(matrix, 0, matrix.size(), false);

    EXPECT_EQ(result[0][0], 4);
    EXPECT_EQ(result[1][0], 3);
    EXPECT_EQ(result[2][0], 2);
    EXPECT_EQ(result[3][0], 1);
}

TEST(SortVector_SortByColumn2) {
    vector<vector<float>> matrix = {
        {0, 0, 300},
        {1, 0, 100},
        {2, 0, 500},
        {3, 0, 200}
    };

    auto result = Sort_Vector(matrix, 2, matrix.size(), false);

    EXPECT_EQ(result[0][2], 500);
    EXPECT_EQ(result[1][2], 300);
    EXPECT_EQ(result[2][2], 200);
    EXPECT_EQ(result[3][2], 100);
}

TEST(SortVector_EqualValues) {
    vector<vector<float>> matrix = {
        {0, 30, 100},
        {1, 30, 200},
        {2, 30, 300}
    };

    auto result = Sort_Vector(matrix, 1, matrix.size(), false);

    EXPECT_EQ(result.size(), static_cast<size_t>(3));
    for (auto& row : result) {
        EXPECT_EQ(row[1], 30);
    }
}

TEST(SortVector_PartialSort) {
    vector<vector<float>> matrix = {
        {0, 10, 0},
        {1, 50, 0},
        {2, 30, 0},
        {3, 60, 0},
        {4, 20, 0}
    };

    auto result = Sort_Vector(matrix, 1, 3, false);

    EXPECT_EQ(result[0][1], 50);
    EXPECT_EQ(result[1][1], 30);
    EXPECT_EQ(result[2][1], 10);
    EXPECT_EQ(result[3][1], 60);
    EXPECT_EQ(result[4][1], 20);
}

// ==================== Edge Case Tests ====================

TEST(EdgeCase_EmptyPops) {
    vector<Pop> pops;
    Market m(&pops);

    m.Stats["Food_produced"] = 10;
    m.Stats["Cloth_produced"] = 10;

    m.Sender();
    m.Sender_cloth();

    EXPECT_EQ(m.Stats["Food_consumed"], 0);
    EXPECT_EQ(m.Stats["Cloth_consumed"], 0);
    EXPECT_EQ(m.Stats["Food_value"], 100);
    EXPECT_EQ(m.Stats["Cloth_value"], 100);
}

TEST(EdgeCase_AllPopsZeroMoneyForFood) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 500, 0, 0, 0, 0, &f, &g);
    Pop b = make_pop(5, 500, 0, 0, 0, 0, &f, &g);
    Pop c = make_pop(3, 500, 0, 0, 0, 0, &f, &g);
    pops.push_back(a);
    pops.push_back(b);
    pops.push_back(c);

    Market m(&pops);
    m.Stats["Food_produced"] = 20;

    m.Sender();

    EXPECT_EQ(m.Stats["Food_consumed"], 18);
    EXPECT_EQ(m.Stats["Food_value"], 0);
}

TEST(EdgeCase_VeryLargeSupply) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 1000, 0, 0, 50, 0, &f, &g);
    pops.push_back(a);

    Market m(&pops);
    m.Stats["Food_produced"] = 100;

    m.Sender();

    EXPECT_EQ(m.Stats["Food_consumed"], 10);
    EXPECT_EQ(m.Stats["Food_value"], 50);
    EXPECT_EQ(m.Pops->at(0).Goods["Food"], 10);
}

TEST(EdgeCase_ClothDemandZero) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(12, 1000, 0, 2, 0, 30, &f, &g);
    Pop b = make_pop(12, 1000, 0, 2, 0, 20, &f, &g);
    pops.push_back(a);
    pops.push_back(b);

    Market m(&pops);
    m.Stats["Cloth_produced"] = 0;
    m.Stats["Cloth_consumed"] = 0;

    m.Sender_cloth();
    m.value_change();

    EXPECT_TRUE(!std::isnan(m.Stats["Cloth_value"]));
}

TEST(EdgeCase_ConsecutiveUpdatesMaintainState) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 3000, 0, 0, 50, 0, &f, &g);
    pops.push_back(a);

    Market m(&pops);

    m.Stats["Food_produced"] = 10;
    m.Sender();
    EXPECT_EQ(m.Stats["Food_consumed"], 10);
    EXPECT_EQ(m.Stats["Food_value"], 50);
    EXPECT_EQ(m.Pops->at(0).Goods["Food"], 10);
    EXPECT_EQ(m.Pops->at(0).money, 3000 - 50 * 10);

    m.Stats["Food_produced"] = 10;
    m.Sender();
    EXPECT_EQ(m.Stats["Food_consumed"], 20);
    EXPECT_EQ(m.Stats["Food_value"], 50);
    EXPECT_EQ(m.Pops->at(0).Goods["Food"], 20);
    EXPECT_EQ(m.Pops->at(0).money, 3000 - 50 * 10 * 2);
}

TEST(EdgeCase_SinglePopGetsAllFoodExact) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(7, 1000, 0, 0, 40, 0, &f, &g);
    pops.push_back(a);

    Market m(&pops);
    m.Stats["Food_produced"] = 7;

    m.Sender();

    EXPECT_EQ(m.Stats["Food_consumed"], 7);
    EXPECT_EQ(m.Pops->at(0).Goods["Food"], 7);
    EXPECT_EQ(m.Pops->at(0).money, 1000 - 40 * 7);
    EXPECT_EQ(m.Stats["Food_value"], 40);
}

TEST(EdgeCase_FoodProducedFloatTruncation) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 1000, 0, 0, 50, 0, &f, &g);
    pops.push_back(a);

    Market m(&pops);
    m.Stats["Food_produced"] = 10.9f;

    m.Sender();

    EXPECT_EQ(m.Stats["Food_consumed"], 10);
}

TEST(EdgeCase_ClothProducedFloatTruncation) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(24, 1000, 0, 0, 0, 50, &f, &g);
    pops.push_back(a);

    Market m(&pops);
    m.Stats["Cloth_produced"] = 1.9f;

    m.Sender_cloth();

    EXPECT_EQ(m.Stats["Cloth_consumed"], 1);
    EXPECT_EQ(m.Pops->at(0).Goods["Cloth"], 1);
}

TEST(EdgeCase_ManyPopsOneBidderWins) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(2, 1000, 0, 0, 10, 0, &f, &g);
    Pop b = make_pop(3, 1000, 0, 0, 90, 0, &f, &g);
    Pop c = make_pop(2, 1000, 0, 0, 30, 0, &f, &g);
    Pop d = make_pop(5, 1000, 0, 0, 50, 0, &f, &g);
    pops.push_back(a);
    pops.push_back(b);
    pops.push_back(c);
    pops.push_back(d);

    Market m(&pops);
    m.Stats["Food_produced"] = 6;

    m.Sender();

    EXPECT_EQ(m.Stats["Food_value"], 50);
    EXPECT_EQ(m.Stats["Food_consumed"], 6);
    EXPECT_EQ(m.Pops->at(1).Goods["Food"], 3);
    EXPECT_EQ(m.Pops->at(3).Goods["Food"], 3);
    EXPECT_EQ(m.Pops->at(2).Goods["Food"], 0);
    EXPECT_EQ(m.Pops->at(0).Goods["Food"], 0);
}

TEST(EdgeCase_UpdateMethodCallsAllThree) {
    Factory f = make_factory();
    Government g = make_gov();
    vector<Pop> pops;

    Pop a = make_pop(10, 2000, 0, 0, 50, 0, &f, &g);
    Pop b = make_pop(5, 2000, 0, 0, 30, 0, &f, &g);
    Pop c = make_pop(24, 2000, 0, 0, 0, 40, &f, &g);
    Pop d = make_pop(12, 2000, 0, 0, 0, 20, &f, &g);
    pops.push_back(a);
    pops.push_back(b);
    pops.push_back(c);
    pops.push_back(d);

    Market m(&pops);
    m.Stats["Food_produced"] = 20;
    m.Stats["Cloth_produced"] = 5;

    float old_food = m.Stats["Food_value"];
    float old_cloth = m.Stats["Cloth_value"];

    m.Update();

    EXPECT_TRUE(m.Stats["Food_consumed"] > 0);
    EXPECT_TRUE(m.Stats["Cloth_consumed"] > 0);
    EXPECT_TRUE(m.Stats["Food_value"] != old_food
                || m.Stats["Cloth_value"] != old_cloth);
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}

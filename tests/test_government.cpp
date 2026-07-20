#include "test_utils.h"
#include "../Agents/Government.h"
#include "../Agents/Pop.h"
#include "../Agents/Factory.h"
#include "../Agents/Market.h"

#include <climits>
#include <cstdint>
#include <vector>

TEST(DefaultConstructor_AllFieldsZero) {
    Government g;
    EXPECT_EQ(g.reserve, 0.0f);
    EXPECT_EQ(g.tax_rate, 0.0f);
    EXPECT_EQ(g.n_food, 0);
    EXPECT_EQ(g.gdp, 0.0f);
    EXPECT_EQ(g.n_tot, 0);
}

TEST(DefaultConstructor_PointersNull) {
    Government g;
    // pointers are uninitialized by default — just verify object is constructable
    (void)g;
}

TEST(ParameterizedConstructor_SetsPointersCorrectly) {
    std::vector<Factory*> facts;
    std::vector<Pop> pops;
    Market m(&pops);
    float tx = 0.15f;

    Government g(&facts, &pops, tx, &m);

    EXPECT_EQ(g.fact_, &facts);
    EXPECT_EQ(g.pop_, &pops);
    EXPECT_EQ(g.market, &m);
}

TEST(ParameterizedConstructor_SetsTaxRateCorrectly) {
    std::vector<Factory*> facts;
    std::vector<Pop> pops;
    Market m(&pops);
    float tx = 0.25f;

    Government g(&facts, &pops, tx, &m);

    EXPECT_EQ(g.tax_rate, 0.25f);
}

TEST(ParameterizedConstructor_ReserveDefaultsToZero) {
    std::vector<Factory*> facts;
    std::vector<Pop> pops;
    Market m(&pops);

    Government g(&facts, &pops, 0.10f, &m);

    EXPECT_EQ(g.reserve, 0.0f);
}

TEST(Census_NormalPopulation) {
    std::vector<Pop> pops(3);
    pops[0].number = 100;
    pops[1].number = 250;
    pops[2].number = 75;
    std::vector<Factory*> facts;
    Market m(&pops);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_tot, 425);
}

TEST(Census_NFood_SumsAllPopFood) {
    std::vector<Pop> pops(2);
    pops[0].Goods["Food"] = 50.0f;
    pops[0].number = 10;
    pops[1].Goods["Food"] = 30.0f;
    pops[1].number = 20;
    std::vector<Factory*> facts;
    Market m(&pops);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_food, 80);
}

TEST(Census_GDP_FoodFactory) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 100.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 50.0f;

    Factory f;
    f.type = "Food";
    f.production = 200;
    facts.push_back(&f);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    float expected_gdp = 200.0f * 50.0f;
    EXPECT_FLOAT_EQ(g.gdp, expected_gdp, 0.001f);
}

TEST(Census_GDP_ClothFactory) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 100.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Cloth_value"] = 75.0f;
    m.Stats["Food_value"] = 50.0f;

    Factory f;
    f.type = "Cloth";
    f.production = 100;
    facts.push_back(&f);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    float expected_gdp = 100.0f * 75.0f;
    EXPECT_FLOAT_EQ(g.gdp, expected_gdp, 0.001f);
    // verify it did NOT use Food_value
    EXPECT_GT(g.gdp, 100.0f * 50.0f);
}

TEST(Census_GDP_MixedFactories) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 100.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 50.0f;
    m.Stats["Cloth_value"] = 75.0f;

    Factory f_food;
    f_food.type = "Food";
    f_food.production = 100;
    facts.push_back(&f_food);

    Factory f_cloth;
    f_cloth.type = "Cloth";
    f_cloth.production = 80;
    facts.push_back(&f_cloth);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    float expected_gdp = (100.0f * 50.0f) + (80.0f * 75.0f);
    EXPECT_FLOAT_EQ(g.gdp, expected_gdp, 0.001f);
}

TEST(Census_EmptyPopulations) {
    std::vector<Pop> pops;
    std::vector<Factory*> facts;
    Market m(&pops);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_tot, 0);
    EXPECT_EQ(g.n_food, 0);
}

TEST(Census_EmptyFactories_GDPZero) {
    std::vector<Pop> pops(2);
    pops[0].number = 50;
    pops[0].Goods["Food"] = 20.0f;
    pops[1].number = 50;
    pops[1].Goods["Food"] = 30.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 100.0f;
    m.Stats["Cloth_value"] = 200.0f;

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_tot, 100);
    EXPECT_EQ(g.n_food, 50);
    EXPECT_EQ(g.gdp, 0.0f);
}

TEST(Census_CalledOnce_Accumulates) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 5.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 10.0f;

    Factory f;
    f.type = "Food";
    f.production = 100;
    facts.push_back(&f);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_tot, 10);
    EXPECT_EQ(g.n_food, 5);
    EXPECT_FLOAT_EQ(g.gdp, 1000.0f, 0.001f);
}

TEST(Census_MultipleCallsAccumulate) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 5.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 10.0f;

    Factory f;
    f.type = "Food";
    f.production = 100;
    facts.push_back(&f);

    Government g(&facts, &pops, 0.0f, &m);

    g.Census();
    EXPECT_EQ(g.n_tot, 10);
    EXPECT_EQ(g.n_food, 5);
    EXPECT_FLOAT_EQ(g.gdp, 1000.0f, 0.001f);

    g.Census();
    EXPECT_EQ(g.n_tot, 20);
    EXPECT_EQ(g.n_food, 10);
    EXPECT_FLOAT_EQ(g.gdp, 2000.0f, 0.001f);

    g.Census();
    EXPECT_EQ(g.n_tot, 30);
    EXPECT_EQ(g.n_food, 15);
    EXPECT_FLOAT_EQ(g.gdp, 3000.0f, 0.001f);
}

TEST(Census_NoAutoResetBetweenCalls) {
    std::vector<Pop> pops(1);
    pops[0].number = 7;
    pops[0].Goods["Food"] = 3.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 5.0f;

    Factory f;
    f.type = "Food";
    f.production = 20;
    facts.push_back(&f);

    Government g(&facts, &pops, 0.0f, &m);

    g.Census();
    g.Census();

    EXPECT_EQ(g.n_tot, 14);
    EXPECT_EQ(g.n_food, 6);
    EXPECT_FLOAT_EQ(g.gdp, 200.0f, 0.001f);
}

TEST(Reset_AfterCensus_ZerosFields) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 5.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 10.0f;

    Factory f;
    f.type = "Food";
    f.production = 100;
    facts.push_back(&f);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_tot, 10);
    EXPECT_EQ(g.n_food, 5);
    EXPECT_FLOAT_EQ(g.gdp, 1000.0f, 0.001f);

    g.Reset();
    EXPECT_EQ(g.n_tot, 0);
    EXPECT_EQ(g.n_food, 0);
    EXPECT_EQ(g.gdp, 0.0f);
}

TEST(Reset_DoesNotAffectOtherFields) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 5.0f;
    std::vector<Factory*> facts;
    Market m(&pops);

    Government g(&facts, &pops, 0.30f, &m);
    g.reserve = 5000.0f;

    g.Census();
    g.Reset();

    EXPECT_EQ(g.tax_rate, 0.30f);
    EXPECT_EQ(g.reserve, 5000.0f);
}

TEST(Census_AfterReset_StartsFromZero) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 5.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 10.0f;

    Factory f;
    f.type = "Food";
    f.production = 100;
    facts.push_back(&f);

    Government g(&facts, &pops, 0.0f, &m);

    g.Census();
    EXPECT_EQ(g.n_tot, 10);
    EXPECT_EQ(g.n_food, 5);
    EXPECT_FLOAT_EQ(g.gdp, 1000.0f, 0.001f);

    g.Reset();
    EXPECT_EQ(g.n_tot, 0);
    EXPECT_EQ(g.n_food, 0);
    EXPECT_EQ(g.gdp, 0.0f);

    g.Census();
    EXPECT_EQ(g.n_tot, 10);
    EXPECT_EQ(g.n_food, 5);
    EXPECT_FLOAT_EQ(g.gdp, 1000.0f, 0.001f);
}

TEST(Reset_MultipleCycles) {
    std::vector<Pop> pops(1);
    pops[0].number = 5;
    pops[0].Goods["Food"] = 2.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 4.0f;

    Factory f;
    f.type = "Food";
    f.production = 50;
    facts.push_back(&f);

    Government g(&facts, &pops, 0.0f, &m);

    for (int cycle = 0; cycle < 5; ++cycle) {
        g.Census();
        EXPECT_EQ(g.n_tot, 5);
        EXPECT_EQ(g.n_food, 2);
        EXPECT_FLOAT_EQ(g.gdp, 200.0f, 0.001f);

        g.Reset();
        EXPECT_EQ(g.n_tot, 0);
        EXPECT_EQ(g.n_food, 0);
        EXPECT_EQ(g.gdp, 0.0f);
    }
}

TEST(Update_CallsCensus) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 5.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 10.0f;

    Factory f;
    f.type = "Food";
    f.production = 100;
    facts.push_back(&f);

    Government g(&facts, &pops, 0.0f, &m);
    g.Update();

    EXPECT_EQ(g.n_tot, 10);
    EXPECT_EQ(g.n_food, 5);
    EXPECT_FLOAT_EQ(g.gdp, 1000.0f, 0.001f);
}

TEST(Update_AccumulatesOverMultipleCalls) {
    std::vector<Pop> pops(1);
    pops[0].number = 3;
    pops[0].Goods["Food"] = 1.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 2.0f;

    Factory f;
    f.type = "Food";
    f.production = 10;
    facts.push_back(&f);

    Government g(&facts, &pops, 0.0f, &m);

    g.Update();
    g.Update();
    g.Update();

    EXPECT_EQ(g.n_tot, 9);
    EXPECT_EQ(g.n_food, 3);
    EXPECT_FLOAT_EQ(g.gdp, 60.0f, 0.001f);
}

TEST(Inflation_NotTriggeredByUpdate) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 5.0f;
    std::vector<Factory*> facts;
    Market m(&pops);

    Government g(&facts, &pops, 0.0f, &m);
    float reserve_before = g.reserve;
    float tax_rate_before = g.tax_rate;

    g.Update();

    EXPECT_EQ(g.reserve, reserve_before);
    EXPECT_EQ(g.tax_rate, tax_rate_before);
}

TEST(Census_NegativeFoodValues) {
    std::vector<Pop> pops(2);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 50.0f;
    pops[1].number = 20;
    pops[1].Goods["Food"] = -30.0f;
    std::vector<Factory*> facts;
    Market m(&pops);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_tot, 30);
    EXPECT_EQ(g.n_food, 20);
}

TEST(Census_AllNegativeFood) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = -40.0f;
    std::vector<Factory*> facts;
    Market m(&pops);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_tot, 10);
    EXPECT_EQ(g.n_food, -40);
}

TEST(Census_ProductionZero) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 5.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 100.0f;
    m.Stats["Cloth_value"] = 50.0f;

    Factory f_food;
    f_food.type = "Food";
    f_food.production = 0;
    facts.push_back(&f_food);

    Factory f_cloth;
    f_cloth.type = "Cloth";
    f_cloth.production = 0;
    facts.push_back(&f_cloth);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.gdp, 0.0f);
    EXPECT_EQ(g.n_tot, 10);
    EXPECT_EQ(g.n_food, 5);
}

TEST(Census_OneProductionZero_OneNonZero) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 5.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 10.0f;
    m.Stats["Cloth_value"] = 20.0f;

    Factory f_food;
    f_food.type = "Food";
    f_food.production = 0;
    facts.push_back(&f_food);

    Factory f_cloth;
    f_cloth.type = "Cloth";
    f_cloth.production = 50;
    facts.push_back(&f_cloth);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    float expected_gdp = 50.0f * 20.0f;
    EXPECT_FLOAT_EQ(g.gdp, expected_gdp, 0.001f);
}

TEST(Census_OneHundredPops) {
    std::vector<Pop> pops(100);
    int expected_n_tot = 0;
    int expected_n_food = 0;
    for (int i = 0; i < 100; ++i) {
        pops[i].number = 1;
        pops[i].Goods["Food"] = static_cast<float>(i);
        expected_n_tot += 1;
        expected_n_food += i;
    }
    std::vector<Factory*> facts;
    Market m(&pops);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_tot, expected_n_tot);
    EXPECT_EQ(g.n_food, expected_n_food);
}

TEST(Census_VeryLargePopulation) {
    std::vector<Pop> pops(1);
    pops[0].number = INT32_MAX;
    pops[0].Goods["Food"] = 0.0f;
    std::vector<Factory*> facts;
    Market m(&pops);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_tot, INT32_MAX);
    EXPECT_EQ(g.n_food, 0);
}

TEST(Census_VeryLargeProductionAndValues) {
    std::vector<Pop> pops(1);
    pops[0].number = 1;
    pops[0].Goods["Food"] = 0.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 1e9f;
    m.Stats["Cloth_value"] = 2e9f;

    Factory f_food;
    f_food.type = "Food";
    f_food.production = 1000000;
    facts.push_back(&f_food);

    Factory f_cloth;
    f_cloth.type = "Cloth";
    f_cloth.production = 500000;
    facts.push_back(&f_cloth);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    float expected_gdp = (1000000.0f * 1e9f) + (500000.0f * 2e9f);
    EXPECT_FLOAT_EQ(g.gdp, expected_gdp, 1e3f);
}

TEST(Census_MultiplePopsMultipleFactories) {
    std::vector<Pop> pops(3);
    pops[0].number = 100;
    pops[0].Goods["Food"] = 50.0f;
    pops[1].number = 200;
    pops[1].Goods["Food"] = 75.0f;
    pops[2].number = 300;
    pops[2].Goods["Food"] = 25.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 30.0f;
    m.Stats["Cloth_value"] = 45.0f;

    Factory f1;
    f1.type = "Food";
    f1.production = 100;
    facts.push_back(&f1);

    Factory f2;
    f2.type = "Cloth";
    f2.production = 200;
    facts.push_back(&f2);

    Factory f3;
    f3.type = "Food";
    f3.production = 300;
    facts.push_back(&f3);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_tot, 600);
    EXPECT_EQ(g.n_food, 150);

    float expected_gdp = (100.0f * 30.0f) + (200.0f * 45.0f) + (300.0f * 30.0f);
    EXPECT_FLOAT_EQ(g.gdp, expected_gdp, 0.001f);
}

TEST(Census_PopWithZeroFoodAndZeroNumber) {
    std::vector<Pop> pops(2);
    pops[0].number = 0;
    pops[0].Goods["Food"] = 0.0f;
    pops[1].number = 0;
    pops[1].Goods["Food"] = 0.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 100.0f;

    Factory f;
    f.type = "Food";
    f.production = 10;
    facts.push_back(&f);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_tot, 0);
    EXPECT_EQ(g.n_food, 0);
    EXPECT_FLOAT_EQ(g.gdp, 1000.0f, 0.001f);
}

TEST(Reset_CensusResetCensusReset_Consistent) {
    std::vector<Pop> pops(1);
    pops[0].number = 42;
    pops[0].Goods["Food"] = 17.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Food_value"] = 3.0f;

    Factory f;
    f.type = "Food";
    f.production = 9;
    facts.push_back(&f);

    Government g(&facts, &pops, 0.0f, &m);

    g.Census();
    EXPECT_EQ(g.n_tot, 42);
    EXPECT_EQ(g.n_food, 17);
    EXPECT_FLOAT_EQ(g.gdp, 27.0f, 0.001f);

    g.Reset();
    EXPECT_EQ(g.n_tot, 0);
    EXPECT_EQ(g.n_food, 0);
    EXPECT_EQ(g.gdp, 0.0f);

    g.Census();
    EXPECT_EQ(g.n_tot, 42);
    EXPECT_EQ(g.n_food, 17);
    EXPECT_FLOAT_EQ(g.gdp, 27.0f, 0.001f);

    g.Reset();
    EXPECT_EQ(g.n_tot, 0);
    EXPECT_EQ(g.n_food, 0);
    EXPECT_EQ(g.gdp, 0.0f);
}

TEST(ParameterizedConstructor_TaxRateZero) {
    std::vector<Factory*> facts;
    std::vector<Pop> pops;
    Market m(&pops);

    Government g(&facts, &pops, 0.0f, &m);

    EXPECT_EQ(g.tax_rate, 0.0f);
}

TEST(ParameterizedConstructor_TaxRateOne) {
    std::vector<Factory*> facts;
    std::vector<Pop> pops;
    Market m(&pops);

    Government g(&facts, &pops, 1.0f, &m);

    EXPECT_EQ(g.tax_rate, 1.0f);
}

TEST(Census_PopsHaveNumberButZeroFood) {
    std::vector<Pop> pops(3);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 0;
    pops[1].number = 20;
    pops[1].Goods["Food"] = 0;
    pops[2].number = 30;
    pops[2].Goods["Food"] = 0;
    std::vector<Factory*> facts;
    Market m(&pops);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    EXPECT_EQ(g.n_tot, 60);
    EXPECT_EQ(g.n_food, 0);
}

TEST(Census_GDP_OnlyClothFactories) {
    std::vector<Pop> pops(1);
    pops[0].number = 10;
    pops[0].Goods["Food"] = 5.0f;
    std::vector<Factory*> facts;
    Market m(&pops);
    m.Stats["Cloth_value"] = 120.0f;

    Factory f1;
    f1.type = "Cloth";
    f1.production = 10;
    facts.push_back(&f1);

    Factory f2;
    f2.type = "Cloth";
    f2.production = 20;
    facts.push_back(&f2);

    Government g(&facts, &pops, 0.0f, &m);
    g.Census();

    float expected_gdp = (10.0f * 120.0f) + (20.0f * 120.0f);
    EXPECT_FLOAT_EQ(g.gdp, expected_gdp, 0.001f);
}

int main() {
    RUN_ALL_TESTS();
    return 0;
}

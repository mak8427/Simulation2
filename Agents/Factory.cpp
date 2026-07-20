//
// Created by dadoi on 22/04/2022.
//
// Factory agent: CES production, posted-price markup, workforce adjustment.
//
// Pricing: Sutton (1991) — firms set markup over unit cost, adjust based
//   on demand signal and competitor prices.
// Workforce: Greenwald & Stiglitz (1993) — firms hire/fire based on
//   sales/production ratio and financial position.
//
#include <math.h>
#include "Factory.h"
#include "Pop.h"
#include "Market.h"
#include <random>
#include <iostream>
#include <stdio.h>

Factory::Factory(Pop* insert_pop, Market* insert_good, Government* insert_gov, nlohmann::json insert_j, int random_int) {
    type = Goods[random_int];
    workers = insert_pop;
    markets = insert_good;
    gov = insert_gov;
    j = insert_j;
    Efficiency_parameter = insert_j[type]["Production_methods"]["PM_1"]["Efficiency_parameter"];
    sigma = insert_j[type]["Production_methods"]["PM_1"]["Sigma"];
    distribution_parameter = insert_j[type]["Production_methods"]["PM_1"]["distribution_parameter"];
    sigma_sostitution = (sigma - 1) / sigma;
    my_price = 10.0f;
    money_gained = salary;  // seed initial Mean_cash so first salary isn't 0
}

// Capital throughput: maps investment to a (0,1] efficiency factor.
float Factory::Troughput(){
    float t = (log(investment+10)/log(10000));
    return t;
};

// CES production: Q = A * [a*K^rho + (1-a)*L^rho]^(1/rho)
void Factory::Production(){
    int workers_count = std::max(1, workers ? workers->number : 0);
    production = Efficiency_parameter*
        pow((distribution_parameter*pow(std::max(1.0f, investment), sigma_sostitution)
           + (1-distribution_parameter)*pow(float(workers_count), sigma_sostitution)),
           1/sigma_sostitution);
    markets->Stats[type+"_produced"] = markets->Stats[type+"_produced"] + production;
    stock_sold = 0;
    // money_gained from previous iteration persists — updated by Market::ClearMarket
};

// Salary = rolling average of revenue, capped by available cash.
// The factory cannot pay more than it has earned + current revenue.
void Factory::Salary(){
    float target = Mean_cash_12_months();
    if (target < 0) target = money_gained;
    if (target < 0) target = 0;
    salary = std::min(target, cash + money_gained);
    if (salary < 0) salary = 0;
    cash = cash - salary;
    if (cash < 0) cash = 0;
};

float Factory::salary_(){
    return salary;
};

// Sliding 12-month average: ring buffer with O(1) insert and O(1) mean.
float Factory::Mean_cash_12_months(){
    if (cash_ring_count < 12) {
        cash_12_months.push_back(money_gained);
        cash_ring_sum += money_gained;
        cash_ring_count++;
    } else {
        cash_ring_sum -= cash_12_months[cash_ring_index];
        cash_ring_sum += money_gained;
        cash_12_months[cash_ring_index] = money_gained;
        cash_ring_index = (cash_ring_index + 1) % 12;
    }
    return cash_ring_sum / cash_ring_count;
}

// Posted-price: adjust toward market-clearing price based on demand signal.
// Price is market-driven, not cost-driven — breaks the salary→price→salary loop.
// Sutton (1991): competitive firms are price-takers, adjusting to market.
void Factory::CalculatePrice(){
    float market_price = markets->Stats[type+"_value"];
    if (market_price <= 0) market_price = my_price;

    // Demand signal: 0-1, how much of our production was sold
    float demand_signal = (production > 0) ? stock_sold / production : 1.0f;

    // Target: market price, nudged by our own demand
    float target = market_price;
    if (demand_signal > 0.95f) target *= 1.02f;      // high demand — price up
    else if (demand_signal < 0.70f) target *= 0.98f;  // low demand — price down

    // Smooth adjustment (15% per iteration toward target)
    my_price += 0.15f * (target - my_price);
    if (my_price < 0.01f) my_price = 0.01f;

    // Unit cost for debug only — not used in pricing
    unit_cost = (production > 0) ? salary / production : 0;
}

// Hire from unemployment pool when demand exceeds supply.
// Firing happens naturally via population starvation.
// Greenwald & Stiglitz (1993) — employment adjusts to sales ratio.
void Factory::AdjustWorkforce(vector<Pop>& pops, vector<Pop*>& unemployed){
    if (production <= 0 || workers == nullptr) return;
    float sales_ratio = stock_sold / production;

    if (sales_ratio > 0.95f && !unemployed.empty()) {
        // Near full demand — hire from unemployment pool
        int hire_count = std::min((int)unemployed.size(), 2);
        for (int h = 0; h < hire_count; h++) {
            Pop* hiree = unemployed.back();
            unemployed.pop_back();
            if (hiree->factory != workers->factory) {  // don't re-hire our own ex-workers
                hiree->factory = this;
                hiree->number = std::max(1, workers->number / 4);  // seed new pop
                hiree->money = salary * 2;  // starting bonus
            }
        }
    }
}

void Factory::Update(){
    Factory::Production();
    Factory::Salary();
    // CalculatePrice() called explicitly after market clearing
};

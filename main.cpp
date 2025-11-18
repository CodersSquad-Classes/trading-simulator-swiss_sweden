#include "orderbook.h"
#include <iostream>
#include <random>
#include <thread>
#include <chrono>
#include <algorithm>
#include <map>
#include <atomic>
#include <iomanip>

OrderBook::OrderBook(): next_id(1), next_ts(1) {}

void OrderBook::record_trade(double price, uint64_t qty, Side aggressor, uint64_t ts) {
    trades.push_back({price, qty, aggressor, ts});
    if (trades.size() > 200) trades.erase(trades.begin(), trades.begin() + (trades.size() - 200));
}

uint64_t OrderBook::post_limit_order(Side side, double price, uint64_t qty) {
    std::lock_guard<std::mutex> lock(m);
    uint64_t ts = next_ts++;
    uint64_t id = next_id++;
    Order incoming{ id, side, price, qty, ts };

    if (side == Side::BUY) {
        auto it = sells.begin();
        while (it != sells.end() && incoming.qty > 0 && it->price <= incoming.price) {
            Order match = *it;
            uint64_t trade_qty = std::min(incoming.qty, match.qty);
            double trade_price = match.price;
            incoming.qty -= trade_qty;

            auto erase_it = it++;
            sells.erase(erase_it);
            if (match.qty > trade_qty) {
                Order remain = match;
                remain.qty -= trade_qty;
                sells.insert(remain);
            }
            record_trade(trade_price, trade_qty, Side::BUY, next_ts++);
        }
        if (incoming.qty > 0) {
            buys.insert(incoming);
            return id;
        }
        return 0;
    } else {
        auto it = buys.begin();
        while (it != buys.end() && incoming.qty > 0 && it->price >= incoming.price) {
            Order match = *it;
            uint64_t trade_qty = std::min(incoming.qty, match.qty);
            double trade_price = match.price;
            incoming.qty -= trade_qty;

            auto erase_it = it++;
            buys.erase(erase_it);
            if (match.qty > trade_qty) {
                Order remain = match;
                remain.qty -= trade_qty;
                buys.insert(remain);
            }
            record_trade(trade_price, trade_qty, Side::SELL, next_ts++);
        }
        if (incoming.qty > 0) {
            sells.insert(incoming);
            return id;
        }
        return 0;
    }
}

void OrderBook::cancel_order(uint64_t order_id) {
    std::lock_guard<std::mutex> lock(m);
    for (auto it = buys.begin(); it != buys.end(); ++it) {
        if (it->id == order_id) { buys.erase(it); return; }
    }
    for (auto it = sells.begin(); it != sells.end(); ++it) {
        if (it->id == order_id) { sells.erase(it); return; }
    }
}

std::vector<LevelAgg> OrderBook::top_buys(size_t n) {
    std::lock_guard<std::mutex> lock(m);
    std::map<double, uint64_t, std::greater<double>> agg;
    for (auto const& o: buys) agg[o.price] += o.qty;

    std::vector<LevelAgg> res;
    for (auto const& kv: agg) {
        if (res.size() >= n) break;
        res.push_back({kv.first, kv.second});
    }
    return res;
}

std::vector<LevelAgg> OrderBook::top_sells(size_t n) {
    std::lock_guard<std::mutex> lock(m);
    std::map<double, uint64_t> agg;
    for (auto const& o: sells) agg[o.price] += o.qty;

    std::vector<LevelAgg> res;
    for (auto const& kv: agg) {
        if (res.size() >= n) break;
        res.push_back({kv.first, kv.second});
    }
    return res;
}

std::vector<Trade> OrderBook::recent_trades(size_t n) {
    std::lock_guard<std::mutex> lock(m);
    std::vector<Trade> res;
    size_t start = trades.size() > n ? trades.size() - n : 0;
    for (size_t i = start; i < trades.size(); ++i) res.push_back(trades[i]);
    return res;
}

// ---------- UI ----------
static const char* MOVE_HOME = "\033[H";
static const char* RED = "\033[31m";
static const char* GREEN = "\033[32m";
static const char* YELLOW = "\033[33m";
static const char* RESET = "\033[0m";

void render(OrderBook &ob) {
    std::cout << MOVE_HOME;

    const size_t LEVELS = 10;
    auto buys = ob.top_buys(LEVELS);
    auto sells = ob.top_sells(LEVELS);
    auto trades = ob.recent_trades(10);

    std::cout << "Continuous Limit Order Book (CLOB) — simulated\n\n";
    std::cout << std::setw(20) << "BUYS (price | qty)" << "    "
              << std::setw(20) << "SELLS (price | qty)" << "\n";
    std::cout << "------------------------------------------------------------\n";

    for (size_t i = 0; i < LEVELS; ++i) {
        if (i < buys.size()) {
            std::cout << GREEN << std::setw(10) << std::fixed << std::setprecision(2)
                      << buys[i].price << " | " << buys[i].qty << RESET;
        } else std::cout << std::setw(20) << " ";

        std::cout << "    ";

        if (i < sells.size()) {
            std::cout << RED << std::setw(10) << std::fixed << std::setprecision(2)
                      << sells[i].price << " | " << sells[i].qty << RESET;
        }
        std::cout << "\n";
    }

    std::cout << "\nRecent trades (most recent last):\n";
    for (auto &t: trades) {
        const char* side = (t.aggressor == Side::BUY) ? "BUY " : "SELL";
        std::cout << YELLOW << side << RESET
                  << " price=" << std::fixed << std::setprecision(2) << t.price
                  << " qty=" << t.qty << "\n";
    }

    std::cout << "\n(press Ctrl-C to exit)\n";
    std::cout.flush();
}

int main() {
    OrderBook ob;
    std::atomic<bool> running{true};

    std::cout << "\033[2J";  // clear screen once

    std::thread producer([&](){
        std::mt19937_64 rng(std::random_device{}());
        std::uniform_real_distribution<double> price_delta(-0.5, 0.5);
        std::uniform_int_distribution<int> side_dist(0,1);
        std::uniform_int_distribution<int> qty_dist(1,100);
        double mid = 100.0;
        while (running) {
            Side s = side_dist(rng) == 0 ? Side::BUY : Side::SELL;
            double p = mid + price_delta(rng);
            uint64_t q = qty_dist(rng);
            ob.post_limit_order(s, std::round(p*100.0)/100.0, q);
            std::this_thread::sleep_for(std::chrono::milliseconds(120 + (rng()%200)));
            if ((rng()%50) == 0) mid += price_delta(rng)*2.0;
        }
    });

    while (true) {
        render(ob);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    running = false;
    producer.join();
    return 0;
}

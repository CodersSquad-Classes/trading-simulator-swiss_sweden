#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <set>
#include <vector>
#include <string>
#include <cstdint>
#include <mutex>

enum class Side { BUY, SELL };

struct Order {
    uint64_t id;
    Side side;
    double price;
    uint64_t qty;
    uint64_t ts;
};

struct LevelAgg {
    double price;
    uint64_t qty;
};

struct Trade {
    double price;
    uint64_t qty;
    Side aggressor;
    uint64_t ts;
};

struct BuyCmp {
    bool operator()(Order const& a, Order const& b) const {
        if (a.price != b.price) return a.price > b.price;
        return a.ts < b.ts;
    }
};

struct SellCmp {
    bool operator()(Order const& a, Order const& b) const {
        if (a.price != b.price) return a.price < b.price;
        return a.ts < b.ts;
    }
};

class OrderBook {
public:
    OrderBook();
    uint64_t post_limit_order(Side side, double price, uint64_t qty);
    void cancel_order(uint64_t order_id);
    std::vector<LevelAgg> top_buys(size_t n);
    std::vector<LevelAgg> top_sells(size_t n);
    std::vector<Trade> recent_trades(size_t n);



#endif

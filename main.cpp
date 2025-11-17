#include <set>
#include <vector>
#include <mutex>

enum class Side { BUY, SELL };

struct Order {
    uint64_t id;
    Side side;
    double price;
    uint64_t qty;
};

struct Trade {
    double price;
    uint64_t qty;
    Side aggressor;
};

struct LevelAgg {
    double price;
    uint64_t qty;
};

struct OrderCmpBuy {
    bool operator()(Order const& a, Order const& b) const {
        if (a.price != b.price) return a.price > b.price;
        return a.id < b.id;
    }
};

struct OrderCmpSell {
    bool operator()(Order const& a, Order const& b) const {
        if (a.price != b.price) return a.price < b.price;
        return a.id < b.id;
    }
};

class OrderBook {
public:
    uint64_t post_limit_order(Side side, double price, uint64_t qty) {
        std::lock_guard<std::mutex> lock(m);
        uint64_t id = ++next_id;
        Order o{ id, side, price, qty };
        if (side == Side::BUY) buys.insert(o);
        else sells.insert(o);
        return id;
    }

    void cancel_order(uint64_t id) {
        std::lock_guard<std::mutex> lock(m);
        for (auto it = buys.begin(); it != buys.end(); ++it)
            if (it->id == id) { buys.erase(it); return; }
        for (auto it = sells.begin(); it != sells.end(); ++it)
            if (it->id == id) { sells.erase(it); return; }
    }

    std::vector<LevelAgg> top_buys(size_t n) {
        std::vector<LevelAgg> r;
        for (auto const& o : buys) {
            r.push_back({o.price, o.qty});
            if (r.size() >= n) break;
        }
        return r;
    }

    std::vector<LevelAgg> top_sells(size_t n) {
        std::vector<LevelAgg> r;
        for (auto const& o : sells) {
            r.push_back({o.price, o.qty});
            if (r.size() >= n) break;
        }
        return r;
    }

private:
    std::multiset<Order, OrderCmpBuy> buys;
    std::multiset<Order, OrderCmpSell> sells;
    uint64_t next_id = 0;
    std::mutex m;
};

int main() {
    OrderBook ob;
    ob.post_limit_order(Side::BUY, 100.0, 10);
    ob.post_limit_order(Side::SELL, 101.0, 5);
}

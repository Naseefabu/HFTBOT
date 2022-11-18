#pragma once
#include "types.hpp"
#include <set>


class OrderBook(){
    std::set<OrderBookRow> bid_book;
    std::set<OrderBookRow> ask_book;
    double best_bid;
    double best_ask;

    OrderBook();
    void apply_diffs(std::vector<OrderBookRow> bids, std::vector<OrderBookRow> asks);
    void apply_snapshot(std::vector<OrderBookRow> bids, std::vector<OrderBookRow> asks);
    double get_price();
};

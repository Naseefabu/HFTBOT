#pragma once
#include "types.hpp"
#include <set>


class OrderBook(){
    std::set<OrderBookEntry> bid_book;
    std::set<OrderBookEntry> ask_book;
    double best_bid;
    double best_ask;

    OrderBook();
    void apply_diffs(std::vector<OrderBookEntry> bids, std::vector<OrderBookEntry> asks);
    void apply_snapshot(std::vector<OrderBookEntry> bids, std::vector<OrderBookEntry> asks);
    double get_price();
    
}
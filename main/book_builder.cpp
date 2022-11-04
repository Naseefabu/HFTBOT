#include "book_builder.hpp"


class OrderBook(){
    std::set<OrderBookEntry> bid_book;
    std::set<OrderBookEntry> ask_book;
    double best_bid;
    double best_ask;
    
}
#pragma once

struct OrderBookMessage
{
    bool is_bid;
    double price;
    double quantity;
    double update_id;
    OrderBookMessage(bool is, double p, double q, double u) : is_bid(is),price(p),quantity(q),update_id(u){}
    OrderBookMessage(){}
};

struct OrderBookRow
{
    double price;
    double quantity;
    double update_id;
};
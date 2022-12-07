#pragma once
#include "TapeSpeedIndicator.hpp"
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

// class MarketSnapshot {
// public:
//   // Construct a MarketSnapshot for a given exchange/pair
//   MarketSnapshot(const std::string& exchange, const std::string& pair)
//       : exchange_(exchange), pair_(pair) {}

//   // Get the exchange/pair associated with this MarketSnapshot
//   std::string exchange() const { return exchange_; }
//   std::string pair() const { return pair_; }

//   // Add a tick event to the MarketSnapshot
//   void update_snapshot(double price, double volume) {
//     ticks_.emplace_back(price, volume);
//   }

//   // Get the latest tick event from the MarketSnapshot
//   std::pair<double, double> latest_tick() const {
//     if (ticks_.empty()) {
//       throw std::runtime_error("MarketSnapshot is empty");
//     }
//     return ticks_.back();
//   }

// private:
//   // The exchange/pair associated with this MarketSnapshot
//   std::string exchange_;
//   std::string pair_;

//   // A vector of tick events, where each tick event is a pair of (price, volume)
//   std::vector<std::pair<double, double>> ticks_;
//   TapeSpeedIndicator tape_speed;
// };
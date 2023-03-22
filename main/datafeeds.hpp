#include "binance-ws.hpp"
#include "binance-http.hpp" 
#include "ftx-http.hpp"
#include "ftx-ws.hpp"
#include "coinbase-ws.hpp"
#include "coinbase-http.hpp"
#include "kraken-http.hpp"
#include "kraken-ws.hpp"
#include <tuple>
#include "types.hpp"
#include <sstream>

template<class T>
class ExchangeOrderbookFeed{
public:
    std::map<std::tuple<std::string,std::string>,Orderbook> orderbook_queues;
    //ExchangeOrderbookFeed()=default;
    void add_queue(std::string exchange, std::string symbol){
        std::cout << "base add queue" << std::endl;

    }
    void build_orderbook(){
        static_cast<T*>(this)->build_ob();
    }

    void build_orderbooks_on_core(){
        std::cout << "base on_core" << std::endl;

    }

};

class BinanceOrderbookFeed : public ExchangeOrderbookFeed<BinanceOrderbookFeed>{

public:
    BinanceOrderbookFeed(){
        std::cout << "binance OB feed" << std::endl;
    }
    void build_ob(){
        std::cout << "build binance book" << std::endl;

    }

};


class KrakenOrderbookFeed : public ExchangeOrderbookFeed<KrakenOrderbookFeed>{

public:
    KrakenOrderbookFeed(){
        std::cout << "kraken ob feed" << std::endl;
    }
    void build_ob(){
        std::cout << "build kraken book" << std::endl;

    }

};

class CoinbaseOrderbookFeed : public ExchangeOrderbookFeed<CoinbaseOrderbookFeed>{
public:
    CoinbaseOrderbookFeed(){
        std::cout << "coinbase OB feed " << std::endl;
    }

    void build_ob(){
        std::cout << "build coinbase book" << std::endl;
    }

};


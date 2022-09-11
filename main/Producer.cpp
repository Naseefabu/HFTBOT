#include "Producer.hpp"

// Producers putting data onto the respective queues.

void binance_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol)
{
    
    auto binancews = std::make_shared<binanceWS>(ioc,ctx);  // Subscribe diff messages
    binancews->subscribe_orderbook_diffs("subscribe",symbol,10);

    while true{
        
    }


}

void ftx_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol)
{
    auto ftxws = std::make_shared<ftxWS>(ioc,ctx);
    ftxws->subscribe_orderbook_diffs("subscribe",symbol);

    while true{

    }

}

void coinbase_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol)
{
    auto coinbasews = std::make_shared<coinbaseWS>(ioc,ctx);
    coinbasews->subscribe_orderbook_diffs("subscribe",symbol);

    while true{

    }

}

void kraken_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol)
{
    auto krakenws = std::make_shared<krakenWS>(ioc,ctx);
    krakenws->subscribe_orderbook_diffs("subscribe",symbol);

    while true{

    }

}

#include "Producer.hpp"
#include "Ringbuffer.hpp"

// Producers putting data onto the respective queues.

void binance_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol)
{
    
    auto binancews = std::make_shared<binanceWS>(ioc,ctx);  // Subscribe diff messages
    binancews->subscribe_orderbook_diffs("subscribe",symbol,10);
    SPSCQueue binance_producer_queue(1000); 



}

void ftx_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol)
{
    auto ftxws = std::make_shared<ftxWS>(ioc,ctx);
    ftxws->subscribe_orderbook_diffs("subscribe",symbol);
    SPSCQueue ftx_producer_queue(1000); 


}

void coinbase_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol)
{
    auto coinbasews = std::make_shared<coinbaseWS>(ioc,ctx);
    coinbasews->subscribe_orderbook_diffs("subscribe",symbol);
    SPSCQueue coinbase_producer_queue(1000); 


}

void kraken_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol, int &levels)
{
    auto krakenws = std::make_shared<krakenWS>(ioc,ctx);
    krakenws->subscribe_orderbook_diffs("subscribe",symbol,levels);
    SPSCQueue kraken_producer_queue(1000); 

}

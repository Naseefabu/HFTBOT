#include "Producer.hpp"
#include "Ringbuffer.hpp"

// FTX api order book data source.py : Hummingbot reference
// Producers putting data onto the respective queues.

void binance_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol)
{
    SPSCQueue<double> binance_producer_queue(1000);
    auto binancews = std::make_shared<binanceWS<double>>(ioc,ctx,binance_producer_queue);  // Subscribe diff messages
    //binanceWS<double> binancews(ioc,ctx,binance_producer_queue);
    binancews->subscribe_orderbook_diffs("subscribe",symbol,10);
     
}

void ftx_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol)
{
    SPSCQueue<double> ftx_producer_queue(1000); 
    auto ftxws = std::make_shared<ftxWS<double>>(ioc,ctx,ftx_producer_queue);
    //ftxWS<double> ftxws(ioc,ctx,ftx_producer_queue);
    ftxws->subscribe_orderbook_diffs("subscribe",symbol);


}

void coinbase_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol)
{
    SPSCQueue<double> coinbase_producer_queue(1000);
    auto coinbasews = std::make_shared<coinbaseWS<double>>(ioc,ctx,coinbase_producer_queue);
    //coinbaseWS<double> coinbasews(ioc,ctx,coinbase_producer_queue);
    coinbasews->subscribe_orderbook_diffs("subscribe",symbol);

}

void kraken_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol, int &levels)
{
    SPSCQueue<double> kraken_producer_queue(1000); 
    auto krakenws = std::make_shared<krakenWS<double>>(ioc,ctx,kraken_producer_queue);
    //krakenWS<double> krakenws(ioc,ctx,kraken_producer_queue);
    krakenws->subscribe_orderbook_diffs("subscribe",symbol,30);
    
}

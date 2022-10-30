#include "Producer.hpp"
#include "Ringbuffer.hpp"
#include "types.hpp"
#include <boost/asio.hpp>

// FTX api order book data source.py : Hummingbot reference
// Producers putting data onto the respective queues.


void binance_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol, int &levels)
{
    SPSCQueue<price_level>binance_producer_queue(1000);
    //auto binancews = std::make_shared<binanceWS<price_level>>(ioc,ctx,binance_producer_queue);  // Subscribe diff messages
    auto binancews = std::make_shared<binanceWS<price_level>>(boost::asio::make_strand(ioc),ctx,binance_producer_queue);
    // binanceWS<double> binancews(ioc,ctx,binance_producer_queue);
    binancews->subscribe_orderbook_diffs("SUBSCRIBE",symbol, levels);
    ioc.run();
}


void ftx_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol)
{
    SPSCQueue<price_level>ftx_producer_queue(1000); 
    auto ftxws = std::make_shared<ftxWS<price_level>>(boost::asio::make_strand(ioc),ctx,ftx_producer_queue);
    //ftxWS<double> ftxws(ioc,ctx,ftx_producer_queue);
    ftxws->subscribe_orderbook_diffs("subscribe",symbol);

    ioc.run();
}


void coinbase_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol)
{
    SPSCQueue<price_level>coinbase_producer_queue(1000);
    auto coinbasews = std::make_shared<coinbaseWS<price_level>>(boost::asio::make_strand(ioc),ctx,coinbase_producer_queue);
    //coinbaseWS<double> coinbasews(ioc,ctx,coinbase_producer_queue);
    coinbasews->subscribe_orderbook_diffs("subscribe",symbol);
    ioc.run();
}


void kraken_producer_main(net::io_context& ioc, ssl::context& ctx, std::string &symbol, int &levels)
{
    SPSCQueue<price_level>kraken_producer_queue(1000); 
    auto krakenws = std::make_shared<krakenWS<price_level>>(boost::asio::make_strand(ioc),ctx,kraken_producer_queue);
    //krakenWS<double> krakenws(ioc,ctx,kraken_producer_queue);
    krakenws->subscribe_orderbook_diffs("subscribe",symbol,30);
    ioc.run();
}

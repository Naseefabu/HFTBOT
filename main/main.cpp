#include <boost/url/src.hpp> // can only include in one source file
#include <chrono>
#include <ctime>
#include <iostream>
#include <new>

#include "TapeSpeedIndicator.hpp"
#include "datafeeds.hpp"

int main()
{
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};

    ctx.set_verify_mode(ssl::verify_peer);
    ctx.set_default_verify_paths();

    std::string coinbase_symbol = "ETH-USD";
    std::string binance_symbol = "btcusdt";
    std::string ftx_symbol = "BTC-PERP";
    std::string kraken_symbol = "XBT/USD";
    std::string kraken_symbol2 = "XBTUSD";
    int levels = 10;

    // coinbase_producer_main(ioc,ctx,coinbase_symbol);
    // binance_producer_main(ioc,ctx,binance_symbol,levels);
    // ftx_producer_main(ioc,ctx,ftx_symbol);
    // kraken_producer_main(ioc,ctx,kraken_symbol,levels);

    // auto binanceapi = std::make_shared<binanceAPI>(ioc.get_executor(),ctx,ioc);
    // std::string l = "10";
    // json out = binanceapi->orderbook(binance_symbol,l);
    // std::cout << "json response binance : " << out << std::endl;

    auto krakenapi = std::make_shared<KrakenAPI>(ioc.get_executor(), ctx, ioc);
    // std::string l = "25";
    // json out = bitfinexapi->get_snapshot("tBTCUSD",l);
    // std::cout << "json bitfinex output : " << out << std::endl;
    // json out1 = bitfinexapi->place_market_sell("tBTCUSD","1");
    // int id = 10;
    // json out2 = bitfinexapi->cancel_order(id);
    double price = 25000;
    double size = 1;

    //json out2 = krakenapi->submit_limit_order("sell", "XBTUSD", size, price);
    //json out2 = krakenapi->submit_market_order("sell", "XBTUSD", size);
    //json out2 = krakenapi->cancel_order(size);
    json out2 = krakenapi->get_account_balance();
    
    std::cout << "market order kraken output : " << out2 << std::endl;
    CoinbaseOrderbookFeed cs;

    ioc.run();
}

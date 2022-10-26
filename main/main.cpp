#include <iostream>
#include <ctime>
#include <chrono> 
#include <new>
#include <boost/url/src.hpp> // can only include in one source file
#include "TapeSpeedIndicator.hpp"
#include <Producer.hpp>


int main()
{
    std::cout << "Bot is still in construction, thankyou for your patience " << std::endl;
    net::io_context ioc;
    ssl::context ctx{ssl::context::tlsv12_client};

    ctx.set_verify_mode(ssl::verify_peer);
    ctx.set_default_verify_paths();

    std::string coinbase_symbol = "ETH-USD";
    std::string binance_symbol = "btcusdt";
    std::string ftx_symbol = "BTC-PERP";
    std::string kraken_symbol = "XBT/USD";
    int levels = 10;
    binance_producer_main(ioc,ctx,binance_symbol);


    ioc.run();
}

#include <iostream>
#include <ctime>
#include <chrono> 
#include <new>
#include <boost/url/src.hpp> // can only include in one source file
#include "TapeSpeedIndicator.hpp"
#include <sstream>
#include <Producer.hpp>
int main()
{
    net::io_context ioc;
    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    ctx.set_verify_mode(ssl::verify_peer);
    ctx.set_default_verify_paths();

    std::cout <<"Cross Exchange market making" << std::endl;

    TapeSpeedIndicator feature(60,50);
    double volume = 500;
    feature.add(volume);
    int tapespeed = feature.get_tapespeed();

    std::cout << "Bot is still in construction, thankyou for your patience " << std::endl;
    //auto coin = std::make_shared<coinbaseWS>(ioc,ctx);
    // coin->subscribe("subscribe","BTC-USD","ticker");
    // std::cout << "coinbase response : " << response << std::endl;

    // auto ftx = std::make_shared<ftxWS>(ioc,ctx);
    // ftx->levelone("subscribe","BTC-PERP");

    // Database d;

    // SHOW_ORDERBOOK();
    auto clientfact = ClientAPI("binance",ioc,ctx);
    auto binance = clientfact.get_exchange_client();
    json payload1 = binance->place_order("BTCUSDT",29500,"BUY","GTC","10");
    // json payload2 = binance->bidask("BTCUSDT");
    // json payload3 = binance->server_time();

    std::cout << "payload 1 : " << payload1 << std::endl;
    // std::cout << "payload 2 : " << payload2 << std::endl;
    //std::cout << "payload 3 : " << payload3 << std::endl;
    
    // t2 = high_resolution_clock::now();
    // // std::cout << payload2 <<std::endl;
    // ms_int = duration_cast<milliseconds>(t2 -t1);
    // std::cout << "it took : " << ms_int.count() << "ms" <<std::endl;


    ioc.run();
}

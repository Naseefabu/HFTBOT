#include <iostream>
#include <ctime>
#include <chrono> 
#include <boost/url/src.hpp> // can only include in one source file
#include "binance-ws.hpp"
#include "binance-http.hpp" 
#include "ftx-http.hpp"
#include "ftx-ws.hpp"
#include "coinbase-http.hpp"
#include <bitset>
#include <sstream>
using json = nlohmann::json;



int main()
{
    net::io_context ioc;
    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    ctx.set_verify_mode(ssl::verify_peer);
    ctx.set_default_verify_paths();
    coinbaseAPI coin(ioc.get_executor(),ctx,ioc);
    json response = coin.place_limit_order("BTC-USD",40000,"45","buy");
    std::cout << "coinbase response : " << response << std::endl;


    auto binance = std::make_shared<binanceAPI>(ioc.get_executor(),ctx,ioc);
    // t1 = high_resolution_clock::now();
    json payload1 = binance->place_order("BTCUSDT",29500,"BUY","GTC","10");
    json payload2 = binance->bidask("BTCUSDT");
    json payload3 = binance->server_time();

    // std::cout << "payload 1 : " << payload1 << std::endl;
    // std::cout << "payload 2 : " << payload2 << std::endl;
    // std::cout << "payload 3 : " << payload3 << std::endl;
    
    // t2 = high_resolution_clock::now();
    // // std::cout << payload2 <<std::endl;
    // ms_int = duration_cast<milliseconds>(t2 -t1);
    // std::cout << "it took : " << ms_int.count() << "ms" <<std::endl;


    ioc.run();
}

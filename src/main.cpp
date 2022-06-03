#include <iostream>
#include <ctime>
#include <chrono> 
#include <boost/url/src.hpp> // can only include in one source file
#include "binance-ws.hpp"
#include "binance-http.hpp" 
#include "ftx-http.hpp"
#include "ftx-ws.hpp"

using json = nlohmann::json;

int main()
{
    net::io_context ioc;
    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    ctx.set_verify_mode(ssl::verify_peer);
    ctx.set_default_verify_paths();

    // WSClient ws(ioc,ctx);
    // ws.orderbook("SUBSCRIBE","btcusdt");
    // auto ws = std::make_shared<ftxWS>(ioc,ctx);
    // ws->trades("subscribe","BTC-PERP");
    // ftxAPI ftx(ioc.get_executor(),ctx,ioc);
    // auto t1 = high_resolution_clock::now();
    // json ftxpayload = ftx.place_order("BTC/USD","buy",30000,10,false,false,false);
    
    // std::cout << ftxpayload << std::endl;
    // auto t2 = high_resolution_clock::now();
    // auto ms_int = duration_cast<milliseconds>(t2 -t1);
    // std::cout << "it took ftx: " << ms_int.count() << "ms" <<std::endl;

    binanceAPI binance(ioc.get_executor(),ctx,ioc);
    // t1 = high_resolution_clock::now();
    json payload1 = binance.place_order("BTCUSDT",29500,e_side::buy,order_type::limit,timeforce::GTC,"10");
    json payload2 = binance.place_order("BTCUSDT",e_side::buy,"10");

    std::cout << "payload 1 : " << payload1 << std::endl;
    std::cout << "payload 2 : " << payload2 << std::endl;
    
    // t2 = high_resolution_clock::now();
    // // std::cout << payload2 <<std::endl;
    // ms_int = duration_cast<milliseconds>(t2 -t1);
    // std::cout << "it took : " << ms_int.count() << "ms" <<std::endl;


    ioc.run();
}

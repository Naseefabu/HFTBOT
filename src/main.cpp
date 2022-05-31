#include <iostream>
#include <ctime>
#include <chrono> 
#include "binance-ws.hpp"
#include "binance-http.hpp" 
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

    binanceAPI binance(ioc.get_executor(),ctx,ioc);

    // httpclients->avg_price("BTCUSDT",operation::asynchronous);
    // json payload1 = httpclients->new_order("BTCUSDT",29500,e_side::buy,order_type::limit,timeforce::GTC,"10");
    // json payload2 = binance.place_order("BTCUSDT",29500,e_side::buy,order_type::limit,timeforce::GTC,"10");
    auto t1 = high_resolution_clock::now();
    json payload2 = binance.place_order("BTCUSDT",29500,e_side::buy,order_type::limit,timeforce::GTC,"10");
    auto t2 = high_resolution_clock::now();
    std::cout << payload2 <<std::endl;
    auto ms_int = duration_cast<milliseconds>(t2 -t1);
    std::cout << "it took : " << ms_int.count() << "ms" <<std::endl;


    ioc.run();
}

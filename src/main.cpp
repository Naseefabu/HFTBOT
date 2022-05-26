#include <iostream>
#include <ctime>
#include <chrono>
#include "boost/url/src.hpp" // can only be included in one source file
#include "binance-ws.hpp"
#include "binance-http.hpp" 
using json = nlohmann::json;


using namespace binance;

int main()
{
    net::io_context ioc;
    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);
    ctx.set_default_verify_paths();

    //auto wsclient = std::make_shared<ws::WebsocketClient>(ioc,ctx);
    //wsclient->full_deltas("SUBSCRIBE","btcusdt",ioc,ctx);

    RESTClient binance(ioc.get_executor(),ctx,ioc);

    // httpclients->avg_price("BTCUSDT",operation::asynchronous);
    // json payload1 = httpclients->new_order("BTCUSDT",29500,e_side::buy,order_type::limit,timeforce::GTC,"10");
    
    auto t1 = high_resolution_clock::now();
    json payload2 = binance.bidask("BTCUSDT");
    std::cout << payload2 <<std::endl;
    auto t2 = high_resolution_clock::now();
    //auto ms_int = duration_cast<milliseconds>(t2 -t1);
    //std::cout << "it took : " << ms_int.count() << "ms" <<std::endl;

    
    
    //std::cout << "my time :" << ts << std::endl;
    //std::cout << "server time :" << payload2 << std::endl;
    // auto t2 = high_resolution_clock::now();


    //std::cout << "it took : " << ms_int.count() << "ms" <<std::endl;
    
    
    
    
    //std::cout << "result payload : "<< payload2 <<std::endl; 
    
    // httpclients->openOrders(operation::synchronous);
    // httpclients->cancel_all_orders("BTCUSDT",operation::synchronous);
    // httpclients->bidask("BTCUSDT",operation::asynchronous);
    // The session is constructed with a strand to
    // ensure that handlers do not execute concurrently.
    

    // rest::cancel_order("BTCUSDT",28,ioc,ctx,operation::asynchronous);
    
    // rest::new_order("BTCUSDT",29500,e_side::buy,order_type::limit,timeforce::GTC,"10",ioc,ctx,binapi::operation::asynchronous);
    // rest::get_account_info(ioc,ctx,operation::asynchronous);
    // rest::openOrders(ioc,ctx,sync);

    // http::response<http::string_body> res = binapi::rest::sync_bidask("BTCUSDT",ioc,ctx);
    // std::cout << res << std::endl;

    // binapi::rest::openOrders(ioc,ctx,async);


    ioc.run();
}

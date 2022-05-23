#include <iostream>
#include <ctime>
#include "boost/url/src.hpp" // can only be included in one source file
#include "binance-ws.hpp"
#include "binance-http.hpp" 

using namespace binapi;

int main()
{
    net::io_context ioc;
    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);
    ctx.set_default_verify_paths();

    auto wsclient = std::make_shared<ws::WebsocketClient>(ioc,ctx);
    wsclient->full_deltas("SUBSCRIBE","btcusdt",ioc,ctx);

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

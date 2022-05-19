#include <iostream>
#include <ctime>
#include "boost/url/src.hpp" // can only be included in one source file
#include "binance-ws.hpp"
#include "binance-http.hpp" 

int main()
{
    net::io_context ioc;
    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);
    ctx.set_default_verify_paths();
    
    binapi::rest::neworder("BTCUSDT",29500,binapi::e_side::buy,binapi::order_type::limit,binapi::timeforce::GTC,"10",ioc,ctx,binapi::operation::synchronous);
    // binapi::rest::openOrders(ioc,ctx,sync);

    // http::response<http::string_body> res = binapi::rest::sync_bidask("BTCUSDT",ioc,ctx);
    // std::cout << res << std::endl;

    // binapi::rest::openOrders(ioc,ctx,async);


    ioc.run();
}

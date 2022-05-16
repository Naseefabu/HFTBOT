#include <iostream>
#include <ctime>
#include "boost/url/src.hpp" // can only be included in one source file
#include "binance-ws.hpp"
#include "binance-gateway.hpp" 

int main()
{
    net::io_context ioc;
    operation sync = synchronous;
    operation async = asynchronous; 
    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);
    ctx.set_default_verify_paths();
    
    auto client = std::make_shared<binapi::rest::httpClient>(ioc.get_executor(),ctx);

    // http::response<http::string_body> res = binapi::rest::sync_bidask("BTCUSDT",ioc,ctx);
    // std::cout << res << std::endl;

    binapi::rest::latest_price("BTCUSDT",ioc,ctx,async);

    std::string secret_key = "QblqlcBtuOrda7ksCRnQ2YiFF6VG6sMRDIVpIpXv9qFHOuMnbVpqzypSdBtIXIU8";
    std::string query_params = "recvWindow=60000&timestamp=1652518379735";

    std::string sign = binapi::rest::encryptWithHMAC(secret_key.c_str(),query_params.c_str());
    std::cout << sign << std::endl;

    // client->ping_binance(ioc,ctx);
    // client->get_open_orders(ioc,ctx);
    // client->get_orderbook("BTCUSDT","3",ioc,ctx);
    // client->get_recent_trades("BTCUSDT","1",ioc,ctx);
    // client->get_candlestick_data("BTCUSDT","5m",ioc,ctx);
    // client->get_avg_price("BTCUSDT",ioc,ctx);

    ioc.run();
}

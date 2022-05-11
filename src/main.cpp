#include <iostream>
#include <ctime>
#include "binance-ws.hpp"
#include "binance-gateway.hpp" // can only be included in one source file
using namespace std;

int main()
{
    net::io_context ioc;
    binapi::rest::httpClient* client;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);
    ctx.set_default_verify_paths();

    // client->get_latest_price("BTCUSDT",ioc,ctx);
    // client->get_exchange_info("BTCUSDT",ioc,ctx);
    client->get_server_time(ioc,ctx);
    // client->ping_binance(ioc,ctx);
    // client->get_open_orders(ioc,ctx);
    // client->get_orderbook("BTCUSDT","3",ioc,ctx);
    // client->get_recent_trades("BTCUSDT","1",ioc,ctx);
    // client->get_candlestick_data("BTCUSDT","5m",ioc,ctx);
    // client->get_avg_price("BTCUSDT",ioc,ctx);

    ioc.run();
}

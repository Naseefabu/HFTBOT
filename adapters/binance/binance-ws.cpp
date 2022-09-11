#include "binance-ws.hpp"

void fail_ws(beast::error_code ec, char const* what);



binanceWS::binanceWS(net::io_context& ioc, ssl::context& ctx)
    : resolver_(net::make_strand(ioc))
    , ws_(net::make_strand(ioc), ctx)
    , ioc(ioc)
    , ctx(ctx)
{
    
}


void binanceWS::run(char const* host, char const* port, json message, const std::string& stream)

{
    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host)) {
        throw boost::system::system_error(beast::error_code(
            ::ERR_get_error(), net::error::get_ssl_category()));
    }

    host_ = host;
    message_text_ = message.dump();

    resolver_.async_resolve(
        host,
        port,
        beast::bind_front_handler(
            &binanceWS::on_resolve,
            shared_from_this()));
}


void binanceWS::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if(ec)
        return fail_ws(ec, "resolve");

    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(),
                                    host_.c_str())) {
        throw beast::system_error{beast::error_code(
            ::ERR_get_error(), net::error::get_ssl_category())};
    }

    get_lowest_layer(ws_).expires_after(30s);


    beast::get_lowest_layer(ws_).async_connect(
        results,
        beast::bind_front_handler(
            &binanceWS::on_connect,
            shared_from_this()));
}

void binanceWS::on_connect(beast::error_code ec, [[maybe_unused]] tcp::resolver::results_type::endpoint_type ep)
{
    if(ec)
        return fail_ws(ec, "connect");


    // Perform the SSL handshake
    ws_.next_layer().async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(&binanceWS::on_ssl_handshake,
                                    shared_from_this()));
}

void binanceWS::on_ssl_handshake(beast::error_code ec)
{
    if(ec)
        return fail_ws(ec, "ssl_handshake");

    beast::get_lowest_layer(ws_).expires_never();


    ws_.set_option(
        websocket::stream_base::timeout::suggested(
            beast::role_type::client));

    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::request_type& req)
        {
            req.set(http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-async");
        }));

    std::cout << "using host_: " << host_ << std::endl;
    ws_.async_handshake(host_, "/ws/",
        beast::bind_front_handler(
            &binanceWS::on_handshake,
            shared_from_this()));
}

void binanceWS::on_handshake(beast::error_code ec)
{
    if(ec) {
        return fail_ws(ec, "handshake");
    }
    
    std::cout << "Sending : " << message_text_ << std::endl;

    ws_.async_write(
        net::buffer(message_text_),
        beast::bind_front_handler(&binanceWS::on_write, shared_from_this()));
}

void binanceWS::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail_ws(ec, "write");
    // pass the event handlers here 
    ws_.async_read(buffer_,beast::bind_front_handler(&binanceWS::on_message,shared_from_this()));
}

void binanceWS::on_message(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail_ws(ec, "read");

    json payload = json::parse(beast::buffers_to_string(buffer_.cdata()));    

    // Signal generation and Quoting strategies on each tick events   
    // Database d;
    // d.ADDRECORD(123654765,payload["bids"][0],payload["bids"][1],payload["asks"][0],payload["asks"][1]);
    // std::cout << "Received: " << beast::buffers_to_string(buffer_.cdata()) <<std::endl;

    ws_.async_read(buffer_,beast::bind_front_handler(&binanceWS::on_message, shared_from_this()));
}


void binanceWS::on_close(beast::error_code ec)
{
    if(ec)
        return fail_ws(ec, "close");

    std::cout << beast::make_printable(buffer_.data()) << std::endl;
}

void binanceWS::subscribe_aggtrades(const std::string& action,const std::string& symbol)
{
    std::string stream = symbol+"@"+"aggTrade";
    json jv = {
        { "method", action },
        { "params", {stream} },
        { "id", 1 }
    };  
    std::make_shared<binanceWS>(ioc,ctx)->run(host, port,jv, stream);
}

void binanceWS::subscribe_trades(const std::string& action,const std::string& symbol)
{
    std::string stream = symbol+"@"+"trade";
    json jv = {
        { "method", action },
        { "params", {stream} },
        { "id", 1 }
    };
    std::make_shared<binanceWS>(ioc,ctx)->run(host, port,jv, stream);
}

/* stream candle stick every second */
void binanceWS::subscribe_candlestick(const std::string& action,const std::string& symbol,const std::string& interval)
{
    std::string stream = symbol+"@"+"kline_"+interval;
    json jv = {
        { "method", action },
        { "params", {stream} },
        { "id", 1 }
    };
    std::make_shared<binanceWS>(ioc,ctx)->run(host, port,jv, stream);
}

void binanceWS::subscribe_levelone(const std::string& action,const std::string& symbol)
{
    std::string stream = symbol+"@"+"bookTicker";
    json jv = {
        { "method", action },
        { "params", {stream} },
        { "id", 1 }
    };
    std::make_shared<binanceWS>(ioc,ctx)->run(host, port,jv, stream);
}

void binanceWS::subscribe_orderbook_diffs(const std::string action,const std::string symbol,short int depth_levels)
{
    std::string stream = symbol+"@"+"depth"+std::to_string(depth_levels);
    json jv = {
        { "method", action },
        { "params", {stream} },
        { "id", 1 }
    };
    std::make_shared<binanceWS>(ioc,ctx)->run(host, port,jv, stream);
}

void binanceWS::subscribe_orderbook(const std::string& action,const std::string& symbol)
{
    std::string stream = symbol+"@"+"depth";
    json jv = {
        { "method", action },
        { "params", {stream} },
        { "id", 1 }
    };
    std::make_shared<binanceWS>(ioc,ctx)->run(host, port,jv, stream);
}



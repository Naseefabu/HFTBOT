#include "kraken-ws.hpp"


void fail_ws(beast::error_code ec, char const* what);   


krakenWS::krakenWS(net::io_context& ioc, ssl::context& ctx)
    : resolver_(net::make_strand(ioc))
    , ws_(net::make_strand(ioc), ctx)
    , ioc(ioc)
    , ctx(ctx)
{
    
}


void krakenWS::run(json message)
{

    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host)) {
        throw boost::system::system_error(beast::error_code(
            ::ERR_get_error(), net::error::get_ssl_category()));
    }

    host_ = host;

    message_text_ = message.dump();

    resolver_.async_resolve(host,"443",beast::bind_front_handler(&krakenWS::on_resolve,shared_from_this()));

}


void krakenWS::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
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
            &krakenWS::on_connect,
            shared_from_this()));
}

void krakenWS::on_connect(beast::error_code ec, [[maybe_unused]] tcp::resolver::results_type::endpoint_type ep)
{
    if(ec)
        return fail_ws(ec, "connect");


    // Perform the SSL handshake
    ws_.next_layer().async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(&krakenWS::on_ssl_handshake,
                                    shared_from_this()));
}

void krakenWS::on_ssl_handshake(beast::error_code ec)
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


    ws_.async_handshake(host_, "/ws/",
        beast::bind_front_handler(
            &krakenWS::on_handshake,
            shared_from_this()));
}

void krakenWS::on_handshake(beast::error_code ec)
{
    if(ec) {
        return fail_ws(ec, "handshake");
    }
    
    std::cout << "Sending : " << message_text_ << std::endl;
    ws_.async_write(
        net::buffer(message_text_),
        beast::bind_front_handler(&krakenWS::on_write, shared_from_this()));
}

void krakenWS::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail_ws(ec, "write");

    ws_.async_read(buffer_,beast::bind_front_handler(&krakenWS::on_message,shared_from_this()));
}

void krakenWS::on_message(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail_ws(ec, "read");

    std::cout << "Received: " << beast::buffers_to_string(buffer_.cdata()) << std::endl;

    ws_.async_read(buffer_,beast::bind_front_handler(&krakenWS::on_message, shared_from_this()));
}


void krakenWS::on_close(beast::error_code ec)
{
    if(ec)
        return fail_ws(ec, "close");

    std::cout << beast::make_printable(buffer_.data()) << std::endl;
}

void krakenWS::subscribe_trades(std::string action, std::string pair)
{
    json payload = {{"event", action},
                {"pair", {pair}}};

    payload["subscription"]["name"] = "trade";
    run(payload);            
}

void krakenWS::subscribe_ticker(std::string action, std::string pair)
{

    json payload = {{"event", action},
                {"pair", {pair}}};

    payload["subscription"]["name"] = "ticker";
    run(payload);            
}

// valid levels options : 10,25,100,500,1000
void krakenWS::subscribe_orderbook(std::string action, std::string pair, int levels)
{

    json payload = {{"event", action},
                {"pair", {pair}}};

    payload["subscription"]["name"] = "book";
    payload["subscription"]["depth"] = levels;
    run(payload);            
}
#include "coinbase-ws.hpp"


void fail_ws(beast::error_code ec, char const* what);   


coinbaseWS::coinbaseWS(net::io_context& ioc, ssl::context& ctx)
    : resolver_(net::make_strand(ioc))
    , ws_(net::make_strand(ioc), ctx)
    , ioc(ioc)
    , ctx(ctx)
{
    
}


void coinbaseWS::run(json message)
{

    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host)) {
        throw boost::system::system_error(beast::error_code(
            ::ERR_get_error(), net::error::get_ssl_category()));
    }

    host_ = host;

    message_text_ = message.dump();

    resolver_.async_resolve(host,"443",beast::bind_front_handler(&coinbaseWS::on_resolve,shared_from_this()));

}


void coinbaseWS::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
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
            &coinbaseWS::on_connect,
            shared_from_this()));
}

void coinbaseWS::on_connect(beast::error_code ec, [[maybe_unused]] tcp::resolver::results_type::endpoint_type ep)
{
    if(ec)
        return fail_ws(ec, "connect");


    // Perform the SSL handshake
    ws_.next_layer().async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(&coinbaseWS::on_ssl_handshake,
                                    shared_from_this()));
}

void coinbaseWS::on_ssl_handshake(beast::error_code ec)
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
            &coinbaseWS::on_handshake,
            shared_from_this()));
}

void coinbaseWS::on_handshake(beast::error_code ec)
{
    if(ec) {
        return fail_ws(ec, "handshake");
    }
    
    std::cout << "Sending : " << message_text_ << std::endl;
    ws_.async_write(
        net::buffer(message_text_),
        beast::bind_front_handler(&coinbaseWS::on_write, shared_from_this()));
}

void coinbaseWS::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail_ws(ec, "write");

    ws_.async_read(buffer_,beast::bind_front_handler(&coinbaseWS::on_message,shared_from_this()));
}

void coinbaseWS::on_message(beast::error_code ec, std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if(ec)
        return fail_ws(ec, "read");
    // Signal generation and Quoting strategies on each tick events   

    std::cout << "Received: " << beast::buffers_to_string(buffer_.cdata()) << std::endl;

    ws_.async_read(buffer_,beast::bind_front_handler(&coinbaseWS::on_message, shared_from_this()));
}


void coinbaseWS::on_close(beast::error_code ec)
{
    if(ec)
        return fail_ws(ec, "close");

    std::cout << beast::make_printable(buffer_.data()) << std::endl;
}

void coinbaseWS::subscribe(std::string method, std::string market, std::string channel)
{

    json payload = {{"type", method},
                {"product_ids", {market}},
                {"channels", {channel}}};

    run(payload);            
}

void coinbaseWS::subscribe_orderbook_diffs(std::string method, std::string market)
{

    json payload = {{"type", method},
                {"product_ids", {market}},
                {"channels", {"level2"}}};

    run(payload);            
}
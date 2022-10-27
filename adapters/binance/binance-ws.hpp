#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <nlohmann/json.hpp>
#include <sys/types.h>
#include <unistd.h>
#include "Ringbuffer.hpp"
#include <system_error>

namespace beast     = boost::beast;    
namespace http      = beast::http;     
namespace websocket = beast::websocket; 
namespace net       = boost::asio;      
namespace ssl       = net::ssl;
using beast::error_code;

using tcp           = net::ip::tcp; 
using json = nlohmann::json;

using TCPStream = beast::tcp_stream;
using SSLStream = beast::ssl_stream<TCPStream>;
using Stream    = websocket::stream<SSLStream>;

using namespace std::chrono_literals;
void fail_ws(beast::error_code ec, char const* what); 
#define HANDLER(f) beast::bind_front_handler(&binanceWS<A>::f, this->shared_from_this())

template <typename A> class binanceWS : public std::enable_shared_from_this<binanceWS<A>> {
    tcp::resolver      resolver_;
    Stream             ws_;
    beast::flat_buffer buffer_;
    std::string        host_;
    std::string        message_text_;

    std::string           wsTarget_ = "/ws/";
    char const*           host      = "stream.binance.com";
    char const*           port      = "9443";
    SPSCQueue<A>&         diff_messages_queue;
    std::function<void()> on_message_handler;
    // OnMessage on_message_cb;

  public:
    binanceWS(net::any_io_executor ex, ssl::context& ctx, SPSCQueue<A>& q)
        : resolver_(ex)
        , ws_(ex, ctx)
        , diff_messages_queue(q) {}

    void run(char const* host, char const* port, json message, const std::string& streamName) {
        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host)) {
            throw boost::system::system_error(
                error_code(::ERR_get_error(), net::error::get_ssl_category()));
        }

        host_         = host;
        message_text_ = message.dump();
        wsTarget_ += streamName;

        resolver_.async_resolve(host_, port, HANDLER(on_resolve));
    }

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec)
            return fail_ws(ec, "resolve");

        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
            throw beast::system_error{
                error_code(::ERR_get_error(), net::error::get_ssl_category())};
        }

        get_lowest_layer(ws_).expires_after(30s);

        beast::get_lowest_layer(ws_).async_connect(results, HANDLER(on_connect));
    }

    void on_connect(beast::error_code                                           ec,
                    [[maybe_unused]] tcp::resolver::results_type::endpoint_type ep) {
        if (ec)
            return fail_ws(ec, "connect");

        // Perform the SSL handshake
        ws_.next_layer().async_handshake(ssl::stream_base::client, HANDLER(on_ssl_handshake));
    }

    void on_ssl_handshake(beast::error_code ec) {
        if (ec)
            return fail_ws(ec, "ssl_handshake");

        beast::get_lowest_layer(ws_).expires_never();

        ws_.set_option(websocket::stream_base::timeout::suggested(beast::role_type::client));

        ws_.set_option(websocket::stream_base::decorator([](websocket::request_type& req) {
            req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client-async");
        }));

        std::cout << "using host_: " << host_ << std::endl;
        ws_.async_handshake(host_, wsTarget_, HANDLER(on_handshake));
    }

    void on_handshake(beast::error_code ec) {
        if (ec) {
            return fail_ws(ec, "handshake");
        }

        std::cout << "Sending : " << message_text_ << std::endl;

        ws_.async_write(net::buffer(message_text_), HANDLER(on_write));
    }

    void on_write(beast::error_code ec, size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail_ws(ec, "write");

        ws_.async_read(buffer_, HANDLER(on_message));
    }

    void on_message(beast::error_code ec, size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if (ec)
            return fail_ws(ec, "read");

        json payload = json::parse(beast::buffers_to_string(buffer_.cdata()));
        std::cout << payload << std::endl;
        // std::cout << "bids : " << payload["b"] << std::endl;
        // std::cout << "asks : " << payload["a"] << std::endl;

        ws_.async_read(buffer_, [this](beast::error_code ec, size_t n) {
            if (ec)
                return fail_ws(ec, "read");
            std::cout << "Orderbook Updates (" << n << ", " << ec.message() << ")" << std::endl;
            buffer_.clear();
            ws_.async_read(buffer_, HANDLER(on_message));
        });
    }

    void on_close(beast::error_code ec) {
        if (ec)
            return fail_ws(ec, "close");

        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }

    
    void subscribe_aggtrades(const std::string& action,const std::string& symbol)
    {
        std::string stream = symbol+"@"+"aggTrade";
        json jv = {
            { "method", action },
            { "params", {stream} },
            { "id", 1 }
        };  
        run(host, port,jv, stream);
    }

    
    void subscribe_trades(const std::string& action,const std::string& symbol)
    {
        std::string stream = symbol+"@"+"trade";
        json jv = {
            { "method", action },
            { "params", {stream} },
            { "id", 1 }
        };
        run(host, port,jv, stream);
    }


    /* stream candle stick every second */
    
    void subscribe_candlestick(const std::string& action,const std::string& symbol,const std::string& interval)
    {
        std::string stream = symbol+"@"+"kline_"+interval;
        json jv = {
            { "method", action },
            { "params", {stream} },
            { "id", 1 }
        };
        run(host, port,jv, stream);
    }

    
    void subscribe_levelone(const std::string& action,const std::string& symbol)
    {
        std::string stream = symbol+"@"+"bookTicker";
        json jv = {
            { "method", action },
            { "params", {stream} },
            { "id", 1 }
        };
        run(host, port,jv, stream);
    }

    
    void subscribe_orderbook_diffs(const std::string action,const std::string symbol,short int depth_levels)
    {
        std::string stream = symbol+"@"+"depth"+std::to_string(depth_levels);
        on_message_handler = []() { std::cout << "Orderbook Levels Update" << std::endl; };
        json jv = {
            { "method", action },
            { "params", {stream} },
            { "id", 1 }
        };
        run(host, port,jv, stream);
    }

    
    void subscribe_orderbook(const std::string& action,const std::string& symbol)
    {
        std::string stream = symbol+"@"+"depth";
        json jv = {
            { "method", action },
            { "params", {stream} },
            { "id", 1 }
        };
        run(host, port,jv, stream);
    }



};






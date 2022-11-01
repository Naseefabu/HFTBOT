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
#include "types.hpp"

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
#define COINBASE_HANDLER(c) beast::bind_front_handler(&coinbaseWS::c, this->shared_from_this())




class coinbaseWS : public std::enable_shared_from_this<coinbaseWS>
{
    tcp::resolver resolver_;
    Stream ws_;
    beast::flat_buffer buffer_;
    std::string message_text_;
    char const* host = "ws-feed.exchange.coinbase.com";
    std::string wsTarget_ = "/ws/";
    std::string host_;
    SPSCQueue<price_level> &diff_messages_queue;
    std::function<void()> on_message_handler;

  public:

    coinbaseWS(net::any_io_executor ex, ssl::context& ctx, SPSCQueue<price_level>& q)
        : resolver_(ex)
        , ws_(ex, ctx)
        , diff_messages_queue(q) {}

    void run(json message) {
        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host)) {
            throw boost::system::system_error(
                error_code(::ERR_get_error(), net::error::get_ssl_category()));
        }
        host_ = host;
        message_text_ = message.dump();

        resolver_.async_resolve(host_, "443", COINBASE_HANDLER(on_resolve));
    }

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec)
            return fail_ws(ec, "resolve");

        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
            throw beast::system_error{
                error_code(::ERR_get_error(), net::error::get_ssl_category())};
        }

        get_lowest_layer(ws_).expires_after(30s);

        beast::get_lowest_layer(ws_).async_connect(results, COINBASE_HANDLER(on_connect));
    }

    void on_connect(beast::error_code                                           ec,
                    [[maybe_unused]] tcp::resolver::results_type::endpoint_type ep) {
        if (ec)
            return fail_ws(ec, "connect");

        // Perform the SSL handshake
        ws_.next_layer().async_handshake(ssl::stream_base::client, COINBASE_HANDLER(on_ssl_handshake));
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
        ws_.async_handshake(host_, wsTarget_, COINBASE_HANDLER(on_handshake));
    }

    void on_handshake(beast::error_code ec) {
        if (ec) {
            return fail_ws(ec, "handshake");
        }

        std::cout << "Sending : " << message_text_ << std::endl;

        ws_.async_write(net::buffer(message_text_), COINBASE_HANDLER(on_write));
    }

    void on_write(beast::error_code ec, size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail_ws(ec, "write");

        ws_.async_read(buffer_, COINBASE_HANDLER(on_message));
    }

    void on_message(beast::error_code ec, size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);
        if (ec)
            return fail_ws(ec, "read");

        buffer_.clear();
        ws_.async_read(buffer_, [this](beast::error_code ec, size_t n) {
            if (ec)
                return fail_ws(ec, "read");
            on_message_handler();
            buffer_.clear();
            ws_.async_read(buffer_, COINBASE_HANDLER(on_message));
        });
    }

    void on_close(beast::error_code ec) {
        if (ec)
            return fail_ws(ec, "close");

        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }

  
  void subscribe(std::string method, std::string market, std::string channel)
  {

      json payload = {{"type", method},
                  {"product_ids", {market}},
                  {"channels", {channel}}};

      run(payload);            
  }

  
  void subscribe_orderbook_diffs(std::string method, std::string market)
  {
      json payload = {{"type", method},
                  {"product_ids", {market}},
                  {"channels", {"level2"}}};
      
      on_message_handler = [this](){

            json payload = json::parse(beast::buffers_to_string(buffer_.cdata()));
            price_level bid_level;
            price_level ask_level;
            bool is;

            if(payload["changes"][0][0] == "buy"){
                bid_level.is_bid = true;
                bid_level.price = std::stod(payload["changes"][0][1].get<std::string>());
                bid_level.quantity = std::stod(payload["changes"][0][2].get<std::string>());
                is = diff_messages_queue.push(bid_level);
            }
            
            if(payload["changes"][0][0] == "sell"){
                ask_level.is_bid = true;
                ask_level.price = std::stod(payload["changes"][0][1].get<std::string>());
                ask_level.quantity = std::stod(payload["changes"][0][2].get<std::string>());
                is = diff_messages_queue.push(bid_level);
            }

        };

      run(payload);            
  }
};

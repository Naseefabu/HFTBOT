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
#define FTX_HANDLER(d) beast::bind_front_handler(&ftxWS::d, this->shared_from_this())



class ftxWS : public std::enable_shared_from_this<ftxWS>
{
    tcp::resolver resolver_;
    Stream ws_;
    beast::flat_buffer buffer_;
    std::string message_text_;
    char const* host = "ftx.com";
    std::string wsTarget_ = "/ws/";
    std::string host_;
    SPSCQueue<OrderBookMessage> &diff_messages_queue;
    std::function<void()> on_message_handler;

  public:

    ftxWS(net::any_io_executor ex, ssl::context& ctx, SPSCQueue<OrderBookMessage>& q)
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

        resolver_.async_resolve(host_, "443", FTX_HANDLER(on_resolve));
    }

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
        if (ec)
            return fail_ws(ec, "resolve");

        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
            throw beast::system_error{
                error_code(::ERR_get_error(), net::error::get_ssl_category())};
        }

        get_lowest_layer(ws_).expires_after(30s);

        beast::get_lowest_layer(ws_).async_connect(results, FTX_HANDLER(on_connect));
    }

    void on_connect(beast::error_code                                           ec,
                    [[maybe_unused]] tcp::resolver::results_type::endpoint_type ep) {
        if (ec)
            return fail_ws(ec, "connect");

        // Perform the SSL handshake
        ws_.next_layer().async_handshake(ssl::stream_base::client, FTX_HANDLER(on_ssl_handshake));
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
        ws_.async_handshake(host_, wsTarget_, FTX_HANDLER(on_handshake));
    }

    void on_handshake(beast::error_code ec) {
        if (ec) {
            return fail_ws(ec, "handshake");
        }

        std::cout << "Sending : " << message_text_ << std::endl;

        ws_.async_write(net::buffer(message_text_), FTX_HANDLER(on_write));
    }

    void on_write(beast::error_code ec, size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return fail_ws(ec, "write");

        ws_.async_read(buffer_, FTX_HANDLER(on_message));
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
            ws_.async_read(buffer_, FTX_HANDLER(on_message));
        });
    
    }

    void on_close(beast::error_code ec) {
        if (ec)
            return fail_ws(ec, "close");

        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }

  // provides the latest best bid and offer market data
  
  void subscribe_ticker(const std::string& action,const std::string& symbol)
  {
      json jv = {
          { "op", action },
          { "channel", "ticker" },
          { "market", symbol }
      };  
      run(jv);
  }

  
  void subscribe_orderbook_diffs(const std::string& action,const std::string& symbol)
  {
      json jv = {
          { "op", action },
          { "channel", "orderbook" },
          { "market", symbol }
      };  
      
      on_message_handler = [this](){

        json payload =  json::parse(beast::buffers_to_string(buffer_.cdata())); 
        
        // bool is;
        // if(payload["data"]["action"] == "update" && payload.contains(payload["data"]["sells"])){ // error 

        //     for(auto x : payload["data"]["asks"]){
        //         OrderBookMessage level;
        //         level.is_bid = false;
        //         level.price = std::stod(x[0].get<std::string>());
        //         level.quantity = std::stod(x[1].get<std::string>());
        //         is = diff_messages_queue.push(level);
        //     }
        // }

        // if(payload["data"]["action"] == "update" && payload.contains(payload["data"]["bids"])){
        //     for(auto x : payload["data"]["bids"]){
        //         OrderBookMessage level;
        //         level.is_bid = true;
        //         level.price = std::stod(x[0].get<std::string>());
        //         level.quantity = std::stod(x[1].get<std::string>());
        //         is = diff_messages_queue.push(level);
        //     }
        // }

      };


      run(jv);
  }

  
  void subscribe_trades(const std::string& action,const std::string& symbol)
  {
      json jv = {
          { "op", action },
          { "channel", "trades" },
          { "market", symbol }
      };  
      run(jv);
  }


};

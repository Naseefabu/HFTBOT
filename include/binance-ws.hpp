#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <boost/json.hpp>

#include <sys/types.h>
#include <unistd.h>

namespace beast     = boost::beast;    
namespace http      = beast::http;     
namespace websocket = beast::websocket; 
namespace net       = boost::asio;      
namespace ssl       = net::ssl;
using tcp           = net::ip::tcp; 

using Stream = websocket::stream<beast::ssl_stream<beast::tcp_stream>>;
using namespace std::chrono_literals;

namespace binapi{
  namespace ws{

    class WebsocketClient : public std::enable_shared_from_this<WebsocketClient>
    {
        tcp::resolver resolver_;
        Stream ws_;

        beast::flat_buffer buffer_;
        std::string host_;
        std::string message_text_;
        // net::io_context &iocc;
        // ssl::context& ctxx;
        std::string streamName = "/ws/";

      public:

        explicit WebsocketClient(net::io_context& ioc, ssl::context& ctx);

        void run(char const* host, char const* port, boost::json::value message, std::string stream);

        void on_resolve(beast::error_code ec, tcp::resolver::results_type results);

        void on_connect(beast::error_code ec, [[maybe_unused]] tcp::resolver::results_type::endpoint_type ep);

        void on_ssl_handshake(beast::error_code ec);

        void on_handshake(beast::error_code ec);

        void on_write(beast::error_code ec, std::size_t bytes_transferred);

        void on_read(beast::error_code ec, std::size_t bytes_transferred);

        void on_close(beast::error_code ec);

        void aggTrades(std::string action,std::string symbol, net::io_context &ioc, ssl::context &ctx);

        void trades(std::string action,std::string symbol, net::io_context &ioc, ssl::context& ctx);

        void candlestick(std::string action,std::string symbol,std::string interval, net::io_context &ioc, ssl::context& ctx);

        void L1_deltas(std::string action,std::string symbol,net::io_context &ioc, ssl::context& ctx);
    
    };


  }
}


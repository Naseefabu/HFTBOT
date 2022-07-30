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

namespace beast     = boost::beast;    
namespace http      = beast::http;     
namespace websocket = beast::websocket; 
namespace net       = boost::asio;      
namespace ssl       = net::ssl;

using tcp           = net::ip::tcp; 
using json = nlohmann::json;

using Stream = websocket::stream<beast::ssl_stream<beast::tcp_stream>>;
using namespace std::chrono_literals;



class binanceWS : public std::enable_shared_from_this<binanceWS>
{
    tcp::resolver resolver_;
    Stream ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string message_text_;
    net::io_context &ioc;
    ssl::context& ctx;
    std::string streamName = "/ws/";
    char const* host = "stream.binance.com";
    char const* port = "9443";

  public:

    explicit binanceWS(net::io_context& ioc, ssl::context& ctx);

    void run(char const* host, char const* port, json message,const std::string& stream);

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);

    void on_connect(beast::error_code ec, [[maybe_unused]] tcp::resolver::results_type::endpoint_type ep);

    void on_ssl_handshake(beast::error_code ec);

    void on_handshake(beast::error_code ec);

    void on_write(beast::error_code ec, std::size_t bytes_transferred);

    void on_message(beast::error_code ec, std::size_t bytes_transferred);

    void on_close(beast::error_code ec);

    void aggtrades(const std::string& action,const std::string& symbol);

    void trades(const std::string& action,const std::string& symbol);

    void candlestick(const std::string& action,const std::string& symbol,const std::string& interval);

    void levelone(const std::string& action,const std::string& symbol);

    void partial_deltas(const std::string action,const std::string symbol,short int depth_levels);

    void orderbook(const std::string& action,const std::string& symbol);

};






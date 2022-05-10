#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include "boost/url/src.hpp"
#include <iostream>
#include <string>


namespace beast = boost::beast;         
namespace http  = beast::http;          
namespace net   = boost::asio;          
namespace ssl   = boost::asio::ssl;     
using tcp       = boost::asio::ip::tcp; 

using executor = net::any_io_executor; 



boost::url make_url(boost::url_view base_api, boost::url_view method);
void fail_http(beast::error_code ec, char const* what);

namespace binapi{

  namespace rest{

    class httpClient : public std::enable_shared_from_this<httpClient> 
    {
        tcp::resolver                        resolver_;
        beast::ssl_stream<beast::tcp_stream> stream_;
        beast::flat_buffer                buffer_; // (Must persist between reads)
        http::request<http::string_body>  req_;
        http::response<http::string_body> res_;
        net::io_context ioc;
        

      public:

        httpClient(executor ex, ssl::context& ctx);
        ssl::context ctxx{ssl::context::tlsv12_client};



        // Start the asynchronous operation
        void run(boost::url url, http::verb action);

        void on_resolve(beast::error_code ec, tcp::resolver::results_type results);

        void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);

        void on_handshake(beast::error_code ec);

        void on_write(beast::error_code ec, std::size_t bytes_transferred);

        void on_read(beast::error_code ec, std::size_t bytes_transferred);

        void on_shutdown(beast::error_code ec);

        void get_latest_price(std::string symbol, net::io_context &ioc, ssl::context &ctx);

        void get_exchange_info(std::string symbol, net::io_context &ioc, ssl::context &ctx);

        void get_server_time(net::io_context &ioc, ssl::context &ctx);

        void ping_binance(net::io_context &ioc, ssl::context &ctx);

        void get_open_orders(net::io_context &ioc, ssl::context &ctx);

        void get_orderbook(std::string symbol, std::string levels, net::io_context &ioc, ssl::context &ctx);

        void get_recent_trades(std::string symbol, std::string levels, net::io_context &ioc, ssl::context &ctx);

        void get_candlestick_data(std::string symbol,std::string interval, net::io_context &ioc, ssl::context &ctx);

        void get_avg_price(std::string symbol, net::io_context &ioc, ssl::context &ctx);

        void get_best_BA(std::string symbol, net::io_context &ioc, ssl::context &ctx);
    };
  }
}

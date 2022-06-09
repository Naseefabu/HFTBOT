#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <nlohmann/json.hpp>
#include<string>
#include <iostream>
#include <string>
#include <chrono>
#include <exception>
#include "utils.hpp"
#include <boost/url.hpp>

namespace beast = boost::beast;         
namespace http  = beast::http;          
namespace net   = boost::asio;          
namespace ssl   = boost::asio::ssl;     
using tcp       = boost::asio::ip::tcp; 

using json = nlohmann::json;
using executor = net::any_io_executor; 

/*
    Timeforce : GTC,IOC,FOK
    fix expensive smart pointers
*/

class binanceAPI : public std::enable_shared_from_this<binanceAPI> 
{

private:    

    void configure(const std::string &api,const std::string &secret);
    boost::url base_api{"https://api1.binance.com/api/v3/"};
    tcp::resolver resolver_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body>  req_;
    http::response<http::string_body> res_;
    std::string api_key = "v6uhUtse5Ae1Gyz72eMSbUMGw7VUDdd5AnqobMOW1Llzi4snnfP4YCyY9V74PFJ4";
    std::string secret_key = "FW8j4YobD26PVP6QLu0sv4Dv7OzrtfgQKzn8FoIMwGzMW9Y0VmX1DatbLIfXoCHV";
    net::io_context& ioc;
    ssl::context& ctx;
    
public:

    binanceAPI(executor ex, ssl::context& ctx,net::io_context& ioc);

    http::response<http::string_body> http_call(boost::url url, http::verb action);

    std::string authenticate(const char* key, const char* data);

    json server_time();

    json latest_price(const std::string &symbol);

    json exchange_info(const std::string &symbol);

    json get_account_info();

    json check_order_status(const std::string &symbol,int orderid);

    json cancel_all_orders(const std::string &symbol);

    json cancel_order(const std::string &symbol,int orderid);

    json place_order(const std::string &symbol,double price,std::string side,std::string timeforce,const std::string &quantity);

    json place_order(const std::string &symbol,std::string side,const std::string &quantity );

    json open_orders();

    json bidask(const std::string &symbol);

    json avg_price(const std::string &symbol);

    json klines(const std::string &symbol,const std::string &interval);

    json recent_trades(const std::string &symbol,const std::string &levels);

    json orderbook(const std::string &symbol,const std::string &levels);

    json ping_binance();

};

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

class bitfinexAPI : public std::enable_shared_from_this<bitfinexAPI> 
{

private:    

    void configure(const std::string &api,const std::string &secret);
    boost::url public_base_api{"https://api-pub.bitfinex.com/v2/"};
    boost::url private_base_api{"https://api.bitfinex.com/v2/"};
    tcp::resolver resolver_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body>  req_;
    http::response<http::string_body> res_;
    std::string api_key = "YRpmzFgOAeaYGCpHqsRAkRZTgtofziPRJsQ59IMey0z";
    std::string secret_key = "0191uDe4OarxnxstnLVGInZGEqOY05culJMZGNWHEtk";
    net::io_context& ioc;
    ssl::context& ctx;
    std::string PostDecodeString;
    
public:

    bitfinexAPI(executor ex, ssl::context& ctx,net::io_context& ioc);

    http::response<http::string_body> http_call(boost::url url, http::verb action);

    json get_snapshot(const std::string &symbol,const std::string &levels);

    json place_market_buy(std::string symbol,std::string amount);

    json place_market_sell(std::string symbol,std::string amount);


};

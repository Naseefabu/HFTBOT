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
using ordered_json = nlohmann::ordered_json;
using executor = net::any_io_executor; 

class KrakenAPI : public std::enable_shared_from_this<KrakenAPI> 
{

private:    

    void configure(const std::string &api, const std::string& secret);
    boost::url base_api{"https://api.kraken.com/0/"};
    std::string secret_key = "NoLQ23Z/JgKRaVhH3D4yk+bF96wLb/UvTQzyhyP9yZRHCWKl65JSZEYoS5w+NXz+Ez05GG3JvvC/Q7x2n6xXgA==";
    std::string PostDecodeString;
    tcp::resolver resolver_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body>  req_;
    http::response<http::string_body> res_;
    std::string api_key = "KlArnozkEc1QqdjbmHM5Yb7/q2IiEN33OKWkiVCAhBR691OOgJN17qpa";
    net::io_context& ioc;
    ssl::context& ctx;
    std::string sign;
    
public:

    explicit KrakenAPI(executor ex, ssl::context& ctx,net::io_context& ioc);

    http::response<http::string_body> http_call(boost::url url, http::verb action);

    json GET_OHLC(std::string pair, std::string interval);

    json get_orderbook(std::string pair, std::string levels);

    // default = last 1000 trades
    json get_trades(std::string pair, std::string sinceTime);


};

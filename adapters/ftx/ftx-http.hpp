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

class ftxAPI : public std::enable_shared_from_this<ftxAPI> 
{

private:    

    void configure(const std::string &api,const std::string &secret);
    boost::url base_api{"https://ftx.com/api/"};
    tcp::resolver resolver_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body>  req_;
    http::response<http::string_body> res_;
    std::string api_key = "XCBZEitcljAjeCH6ZxkEdn7ngWrMMr8ytTp4GAiX";
    std::string secret_key = "GKPAz77T2X54MQH3ICe379ZFxS_Si9qbxzJtzQ5I";
    net::io_context& ioc;
    ssl::context& ctx;
    std::string sign;
    
public:

    explicit ftxAPI(executor ex, ssl::context& ctx,net::io_context& ioc);

    http::response<http::string_body> http_call(boost::url url, http::verb action);

    std::string authenticate(const char* key, const char* data);

    json list_markets();

    json list_market(std::string market);

    json get_orderbook(std::string market,int depth);

    json get_trades(std::string market);

    json list_future(std::string future);

    json future_stats(std::string future);

    json account_info(std::string future);

    json closed_positions();

    json open_orders();

    json open_orders(std::string market);

    json place_order(std::string market, std::string side, double size,bool ioc,bool post_only,bool reduce_only);

    json cancel_order(int orderid);

    json cancel_all_orders(std::string market);

    json place_order(std::string market, std::string side,double price, double size,bool ioc,bool post_only,bool reduce_only);
};

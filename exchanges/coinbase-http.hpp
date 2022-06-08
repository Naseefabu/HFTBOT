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

class coinbaseAPI : public std::enable_shared_from_this<coinbaseAPI> 
{

private:    

    void configure(const std::string &api, const std::string& secret, const std::string &pass);
    boost::url base_api{"https://api.exchange.coinbase.com/"};
    std::string secret_key = "Fhy8v43DGnUC6wPKF89ImG8xUMjiMGduSKlGaV39Ttbgh6gbk4lNkH3g/GBh3tYkm5gDKCUilyS8WFh0Bn1wVQ==";
    std::string PostDecodeString;
    tcp::resolver resolver_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_;
    http::request<http::string_body>  req_;
    http::response<http::string_body> res_;
    std::string api_key = "db11f28ba4fa14b548ee568c4d236745";
    std::string passphrase = "f717xa2bkqi";
    net::io_context& ioc;
    ssl::context& ctx;
    std::string sign;
    
public:

    explicit coinbaseAPI(executor ex, ssl::context& ctx,net::io_context& ioc);

    std::string authenticate(const char* key, const char* data); 

    http::response<http::string_body> http_call(boost::url url, http::verb action);

    json open_orders();

    json cancel_order(int orderid);

    json cancel_all();

    ordered_json place_market_buy(std::string market,std::string size);

    ordered_json place_market_sell(std::string market,std::string size);

    ordered_json place_limit_order(std::string market,int price,std::string size, std::string side);


};

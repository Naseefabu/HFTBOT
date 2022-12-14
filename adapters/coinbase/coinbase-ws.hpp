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
#include <unordered_map>

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
    SPSCQueue<OrderBookMessage> &diff_messages_queue;
    std::function<void()> on_orderbook_diffs;
    std::unordered_map<double,std::unordered_map<std::string,std::unordered_map<std::string,double>>> bids;
    std::unordered_map<double,std::unordered_map<std::string,std::unordered_map<std::string,double>>> asks;

  public:

    coinbaseWS(net::any_io_executor ex, ssl::context& ctx, SPSCQueue<OrderBookMessage>& q)
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
            on_orderbook_diffs();
            buffer_.clear();
            ws_.async_read(buffer_, COINBASE_HANDLER(on_message));
        });
    }

    void on_close(beast::error_code ec) {
        if (ec)
            return fail_ws(ec, "close");

        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }

  
  void subscribe(const std::string& method, const std::string& market, const std::string& channel)
  {

      json payload = {{"type", method},
                  {"product_ids", {market}},
                  {"channels", {channel}}};

      run(payload);            
  }


    double get_agg_size_for_bids(const double& price){
        double aggsize = 0;
        for(auto x : bids[price]){
            aggsize += x.second["remaining_size"];
        }
        return aggsize;
    }

    double get_agg_size_for_asks(const double& price){
        double aggsize = 0;
        for(auto x : asks[price]){
            aggsize += x.second["remaining_size"];
        }
        return aggsize;
    }

  // level 3 orderbook messages
  void subscribe_orderbook_diffs(const std::string& method, const std::string& market)
  {
      json payload = {{"type", method},
                  {"product_ids", {market}},
                  {"channels", {"full"}}};
      
      on_orderbook_diffs = [this](){

            json payload = json::parse(beast::buffers_to_string(buffer_.cdata()));
            // std::cout << "payload : " << payload << std::endl;

            std::string side = payload["side"];
            std::string msg_type = payload["type"];
            std::string price_raw = payload["price"];
            std::unordered_map<std::string,double> order_dict;
            double price = std::stod(price_raw);
            double newfunds=0;
            double remaining_size=0;
            std::string order_id;
            double agg_size;
            bool is;

            if(payload.contains(payload["order_id"]))
                order_id = payload["order_id"];
            else
                order_id = payload["maker_order_id"];
            
            if(price_raw == "null")
                std::cout << "null" << std::endl;

            if(msg_type == "open"){
                remaining_size = std::stod(payload["remaining_size"].get<std::string>());
                order_dict.insert({"remaining_size",remaining_size});
                if(side == "buy"){
                    if(bids.find(price) != bids.end())
                        bids[price][order_id] = order_dict; // replacing yes
                    else
                        bids[price][order_id] = order_dict;    // add new if not existing
                    agg_size = get_agg_size_for_bids(price); // aggregating size by price
                    is = diff_messages_queue.push(OrderBookMessage(true,price,agg_size,payload["sequence"]));
                }

                else{
                    if(asks.find(price) != asks.end())
                        asks[price][order_id] = order_dict;
                    else
                        asks[price][order_id] = order_dict;
                    agg_size = get_agg_size_for_asks(price);
                    is = diff_messages_queue.push(OrderBookMessage(false,price,agg_size,payload["sequence"]));
                }

            }   

            else if(msg_type == "change"){

                if(payload.contains(payload["new_size"]))
                    remaining_size = std::stod(payload["remaining_size"].get<std::string>());
                else if(payload.contains(payload["new_funds"])){
                    newfunds = std::stod(payload["new_funds"].get<std::string>());
                    remaining_size = newfunds / price;
                }
                else
                    std::cout << "invalid diff message" << std::endl;

                if(side == "buy"){
                    if((bids.find(price) != bids.end()) && (bids[price].find(order_id) != bids[price].end())){
                        bids[price][order_id]["remaining_size"] = remaining_size; 
                        agg_size = get_agg_size_for_bids(price);
                        is = diff_messages_queue.push(OrderBookMessage(true,price,agg_size,payload["sequence"]));
                    }
                    else
                        std::cout << "empty update " << std::endl;
                }

                else{
                    if((asks.find(price) != asks.end()) && (asks[price].find(order_id) != asks[price].end())){
                        asks[price][order_id]["remaining_size"] = remaining_size; 
                        agg_size = get_agg_size_for_asks(price);
                        is = diff_messages_queue.push(OrderBookMessage(false,price,agg_size,payload["sequence"]));
                    }
                    else
                        std::cout << "empty update " << std::endl;
                    
                }    
            } 

            else if(msg_type == "match"){

                if(side == "buy"){
                      if((bids.find(price) != bids.end()) && (bids[price].find(order_id) != bids[price].end())){
                        double size = bids[price][order_id]["remaining_size"]; 
                        bids[price][order_id]["remaining_size"] = size - std::stod(payload["size"].get<std::string>());
                        agg_size = get_agg_size_for_bids(price);
                        is = diff_messages_queue.push(OrderBookMessage(true,price,agg_size,payload["sequence"])); 
                    }
                    else
                        std::cout << "empty update " << std::endl;
                }
                else{
                    if((asks.find(price) != asks.end()) && (asks[price].find(order_id) != asks[price].end())){
                        double size = asks[price][order_id]["remaining_size"];
                        asks[price][order_id]["remaining_size"] = size - std::stod(payload["size"].get<std::string>());
                        agg_size = get_agg_size_for_asks(price);
                        is = diff_messages_queue.push(OrderBookMessage(false,price,agg_size,payload["sequence"]));
                    }
                    else
                        std::cout << "empty update" << std::endl;
                }

            }

            else if(msg_type == "done"){
                if(side == "buy"){
                     if((bids.find(price) != bids.end()) && (bids[price].find(order_id) != bids[price].end())){
                        bids[price].erase(order_id);
                        if(bids[price].size() < 1){ 
                            bids.erase(price); 
                            is = diff_messages_queue.push(OrderBookMessage(true,price,0.0,payload["sequence"]));
                        }
                        else{
                            agg_size = get_agg_size_for_bids(price);
                            is = diff_messages_queue.push(OrderBookMessage(true,price,agg_size,payload["sequence"]));
                        }
                    }
                }
                else{

                   if((asks.find(price) != asks.end()) && (asks[price].find(order_id) != asks[price].end())){
                        asks[price].erase(order_id);
                        if(asks[price].size() < 1){ 
                            asks.erase(price);
                            is = diff_messages_queue.push(OrderBookMessage(false,price,0.0,payload["sequence"]));
                        }
                        else{
                            agg_size = get_agg_size_for_asks(price);
                            is = diff_messages_queue.push(OrderBookMessage(false,price,agg_size,payload["sequence"]));
                        }
                    }
                }
                
            }
            else{
                std::cout << "Invalid Message Type" << std::endl;
            }

    };

    run(payload);            
  }
};



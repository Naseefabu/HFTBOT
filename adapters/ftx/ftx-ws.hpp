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
namespace beast     = boost::beast;    
namespace http      = beast::http;     
namespace websocket = beast::websocket; 
namespace net       = boost::asio;      
namespace ssl       = net::ssl;

using tcp           = net::ip::tcp; 
using json = nlohmann::json;

using Stream = websocket::stream<beast::ssl_stream<beast::tcp_stream>>;
using namespace std::chrono_literals;
void fail_ws(beast::error_code ec, char const* what);   

template<typename D>
class ftxWS : public std::enable_shared_from_this<ftxWS<D>>
{
    tcp::resolver resolver_;
    Stream ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string message_text_;
    net::io_context &ioc;
    ssl::context& ctx;
    std::string streamName = "/ws/";
    char const* host = "ftx.com";
    SPSCQueue<D> &diff_messages_queue;

  public:
  
    
    ftxWS(net::io_context& ioc, ssl::context& ctx, SPSCQueue<D> &q)
        : resolver_(net::make_strand(ioc))
        , ws_(net::make_strand(ioc), ctx)
        , ioc(ioc)
        , ctx(ctx)
        , diff_messages_queue(q)
    {
        
    }

  
  void run(json message)
  {

      if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host)) {
          throw boost::system::system_error(beast::error_code(
              ::ERR_get_error(), net::error::get_ssl_category()));
      }

      host_ = host;
      message_text_ = message.dump();

      resolver_.async_resolve(host,"443",beast::bind_front_handler(&ftxWS<D>::on_resolve,this->shared_from_this()));
  }

  
  void on_resolve(beast::error_code ec, tcp::resolver::results_type results)
  {
      if(ec)
          return fail_ws(ec, "resolve");

      if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(),
                                      host_.c_str())) {
          throw beast::system_error{beast::error_code(
              ::ERR_get_error(), net::error::get_ssl_category())};
      }

      get_lowest_layer(ws_).expires_after(30s);


      beast::get_lowest_layer(ws_).async_connect(
          results,
          beast::bind_front_handler(
              &ftxWS<D>::on_connect,
              this->shared_from_this()));
  }

  
  void on_connect(beast::error_code ec, [[maybe_unused]] tcp::resolver::results_type::endpoint_type ep)
  {
      if(ec)
          return fail_ws(ec, "connect");


      // Perform the SSL handshake
      ws_.next_layer().async_handshake(
          ssl::stream_base::client,
          beast::bind_front_handler(&ftxWS<D>::on_ssl_handshake,
                                      this->shared_from_this()));
  }

  
  void on_ssl_handshake(beast::error_code ec)
  {
      if(ec)
          return fail_ws(ec, "ssl_handshake");

      beast::get_lowest_layer(ws_).expires_never();


      ws_.set_option(
          websocket::stream_base::timeout::suggested(
              beast::role_type::client));

      ws_.set_option(websocket::stream_base::decorator(
          [](websocket::request_type& req)
          {
              req.set(http::field::user_agent,
                  std::string(BOOST_BEAST_VERSION_STRING) +
                      " websocket-client-async");
          }));


      ws_.async_handshake(host_, "/ws/",
          beast::bind_front_handler(
              &ftxWS<D>::on_handshake,
              this->shared_from_this()));
  }

  
  void on_handshake(beast::error_code ec)
  {
      if(ec) {
          return fail_ws(ec, "handshake");
      }
      
      std::cout << "Sending : " << message_text_ << std::endl;

      ws_.async_write(
          net::buffer(message_text_),
          beast::bind_front_handler(&ftxWS<D>::on_write, this->shared_from_this()));
  }

  
  void on_write(beast::error_code ec, std::size_t bytes_transferred) {
      boost::ignore_unused(bytes_transferred);

      if(ec)
          return fail_ws(ec, "write");
      
      ws_.async_read(buffer_,beast::bind_front_handler(&ftxWS<D>::on_message,this->shared_from_this()));
  }

  
  void on_message(beast::error_code ec, std::size_t bytes_transferred)
  {
      // copy gamma ray on message logic 

      boost::ignore_unused(bytes_transferred);

      if(ec)
          return fail_ws(ec, "read");

      json payload =  json::parse(beast::buffers_to_string(buffer_.cdata()));   
      // std::cout << "payload output : " << payload << std::endl;

      if(payload["channel"] == "orderbook" && payload["type"] == "update")
      {
        json bids = payload["data"]["bids"];
        json asks = payload["data"]["asks"];
        std::cout << "Json Bids : " << bids << std::endl;
        std::cout << "Json Asks : " << asks << std::endl;  
        //   double bidsize = payload["data"]["bidSize"].get<float>();
        //   double ask = payload["data"]["asks"].get<float>();
        //   double asksize = payload["data"]["askSize"].get<float>();
        //   bool istrue = diff_messages_queue.push(bid);
        //   std::cout << "output : "<<istrue << std::endl;
      }    

      buffer_.clear();
      ws_.async_read(buffer_,beast::bind_front_handler(&ftxWS<D>::on_message, this->shared_from_this()));
  }

  
  void on_close(beast::error_code ec)
  {
      if(ec)
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

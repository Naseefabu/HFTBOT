#ifndef WEBSOCKET
#define WEBSOCKET

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/json.h>

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


class WebsocketClient : public std::enable_shared_from_this<WebsocketClient>
{
    tcp::resolver resolver_;
    Stream        ws_;

    beast::flat_buffer buffer_;
    std::string        host_;
    std::string        message_text_;

  public:

    explicit WebsocketClient(net::io_context& ioc, ssl::context& ctx);

    void run(char const* host, char const* port, Json::Value message);

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);

    void on_connect(beast::error_code ec, [[maybe_unused]] tcp::resolver::results_type::endpoint_type ep);

    void on_ssl_handshake(beast::error_code ec);

    std::string Json_to_string(const Json::Value& json);

    void on_handshake(beast::error_code ec);

    void on_write(beast::error_code ec, std::size_t bytes_transferred);

    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    void on_close(beast::error_code ec);

};

#endif
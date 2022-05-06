#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <string>


namespace beast = boost::beast;         
namespace http  = beast::http;          
namespace net   = boost::asio;          
namespace ssl   = boost::asio::ssl;     
namespace json  = boost::json;
using tcp       = boost::asio::ip::tcp; 

using executor = net::any_io_executor; 

class httpClient : public std::enable_shared_from_this<httpClient>
{
    tcp::resolver                        resolver_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer                buffer_; // (Must persist between reads)
    http::request<http::string_body>  req_;
    http::response<http::string_body> res_;

  public:

    explicit httpClient(executor ex, ssl::context& ctx);

    // Start the asynchronous operation
    void run(boost::url url);

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);

    void on_handshake(beast::error_code ec);

    void on_write(beast::error_code ec, std::size_t bytes_transferred);

    void on_read(beast::error_code ec, std::size_t bytes_transferred);

    void on_shutdown(beast::error_code ec);
};
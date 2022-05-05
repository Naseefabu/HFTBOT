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


void fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}



class WebsocketClient : public std::enable_shared_from_this<WebsocketClient>
{
    tcp::resolver resolver_;
    Stream        ws_;

    beast::flat_buffer buffer_;
    std::string        host_;
    std::string        message_text_;

  public:

    explicit WebsocketClient(net::io_context& ioc, ssl::context& ctx)
        : resolver_(net::make_strand(ioc))
        , ws_(net::make_strand(ioc), ctx)
    {
    }

    
    void run(char const* host, char const* port, Json::Value message)

    {
        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host)) {
            throw boost::system::system_error(beast::error_code(
                ::ERR_get_error(), net::error::get_ssl_category()));
        }

        host_         = host;
        message_text_ = Json_to_string(message);

        resolver_.async_resolve(
            host,
            port,
            beast::bind_front_handler(
                &WebsocketClient::on_resolve,
                shared_from_this()));
    }


    void on_resolve(beast::error_code ec, tcp::resolver::results_type results)
    {
        if(ec)
            return fail(ec, "resolve");

        // Set SNI Hostname (many hosts need this to ssl handshake successfully)
        if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(),
                                      host_.c_str())) {
            throw beast::system_error{beast::error_code(
                ::ERR_get_error(), net::error::get_ssl_category())};
        }

        // Set the timeout for the operation
        get_lowest_layer(ws_).expires_after(30s);


        beast::get_lowest_layer(ws_).async_connect(
            results,
            beast::bind_front_handler(
                &WebsocketClient::on_connect,
                shared_from_this()));
    }

    void on_connect(beast::error_code ec, [[maybe_unused]] tcp::resolver::results_type::endpoint_type ep)
    {
        if(ec)
            return fail(ec, "connect");


        // Perform the SSL handshake
        ws_.next_layer().async_handshake(
            ssl::stream_base::client,
            beast::bind_front_handler(&WebsocketClient::on_ssl_handshake,
                                      shared_from_this()));
    }

    void on_ssl_handshake(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "ssl_handshake");

        // Turn off the timeout on the tcp_stream, because
        // the websocket stream has its own timeout system.
        beast::get_lowest_layer(ws_).expires_never();

        // Set suggested timeout settings for the websocket
        ws_.set_option(
            websocket::stream_base::timeout::suggested(
                beast::role_type::client));

        // Set a decorator to change the User-Agent of the handshake
        ws_.set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req)
            {
                req.set(http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                        " websocket-client-async");
            }));

        std::cout << "using host_: " << host_ << std::endl;
        ws_.async_handshake(host_, "/ws/btcusdt@depth",
            beast::bind_front_handler(
                &WebsocketClient::on_handshake,
                shared_from_this()));
    }

    std::string Json_to_string(const Json::Value& json) {
        Json::StreamWriterBuilder wbuilder;
        wbuilder["indentation"] = ""; // Optional
        return Json::writeString(wbuilder, json);
    }

    void on_handshake(beast::error_code ec)
    {
        if(ec) {
            return fail(ec, "handshake");
        }
        
        std::cout << "Sending " << message_text_ << std::endl;

        ws_.async_write(
            net::buffer(message_text_),
            beast::bind_front_handler(&WebsocketClient::on_write, shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");
        
        ws_.async_read(
            buffer_,
            beast::bind_front_handler(
                &WebsocketClient::on_read,
                shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        std::cout << "Received: " << beast::buffers_to_string(buffer_.cdata()) <<std::endl;

        ws_.async_read(
            buffer_,
            beast::bind_front_handler(&WebsocketClient::on_read, shared_from_this()));
    }


    void on_close(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "close");

        std::cout << beast::make_printable(buffer_.data()) << std::endl;
    }



};




int main()
{
    std::ifstream file("details.json");
    file.open("details.json");

    if(file.is_open())
    {
        std::cout <<  "json opened" <<std::endl;
    }

    Json::Value actualjson;
    Json::Reader jsonreader;

    file >> actualjson;

    net::io_context ioc;

    ssl::context ctx{ssl::context::tlsv12_client};

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);
    ctx.set_default_verify_paths();


    std::make_shared<WebsocketClient>(ioc, ctx)->run("stream.binance.com", "9443",actualjson);

    ioc.run();
}
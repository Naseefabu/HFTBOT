#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/json.hpp>
#include <boost/json/src.hpp>
#include <functional>
#include <iostream>
#include <memory>
#include <string>

#include "boost/url/src.hpp"
#include "boost/url.hpp"


namespace beast = boost::beast;         
namespace http  = beast::http;          
namespace net   = boost::asio;          
namespace ssl   = boost::asio::ssl;     
namespace json  = boost::json;
using tcp       = boost::asio::ip::tcp; 

using executor = net::any_io_executor; 

//------------------------------------------------------------------------------

boost::url make_url(boost::url_view base_api, boost::url_view method) {
    assert(!method.is_path_absolute());
    assert(base_api.data()[base_api.size() - 1] == '/');

    boost::urls::error_code ec;
    boost::url url;
    resolve(base_api, method, url, ec);
    if (ec)
        throw boost::system::system_error(ec);
    return url;
}
// Report a failure
void fail(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

// Performs an HTTP GET and prints the response
class session : public std::enable_shared_from_this<session>
{
    tcp::resolver                        resolver_;
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer                buffer_; // (Must persist between reads)
    http::request<http::string_body>  req_;
    http::response<http::string_body> res_;

  public:
    explicit session(executor ex, ssl::context& ctx)
        : resolver_(ex)
        , stream_(ex, ctx) {}

    // Start the asynchronous operation
    void run(boost::url url) {

        std::string const host(url.host());
        std::string const service = url.has_port() //
            ? url.port()
            : (url.scheme_id() == boost::urls::scheme::wss) //
                ? "https"
                : "http";
        url.remove_origin(); // becomes req_.target()

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(! SSL_set_tlsext_host_name(stream_.native_handle(), host.c_str()))
        {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            std::cerr << ec.message() << "\n";
            return;
        }

        // Set up an HTTP GET request message
        // req_.version(version);
        req_.method(http::verb::get);
        req_.target(url.c_str());
        req_.set(http::field::host, host);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        //req_.body() = serialize(json::object {{"symbol", "btcusdt"}});
        req_.prepare_payload(); // make HTTP 1.1 compliant

        // Look up the domain name

        resolver_.async_resolve(host, service,
                                beast::bind_front_handler(&session::on_resolve,
                                                          shared_from_this()));
    }

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results)
    {
        if(ec)
            return fail(ec, "resolve");

        // Set a timeout on the operation
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(stream_).async_connect(
            results,
            beast::bind_front_handler(
                &session::on_connect,
                shared_from_this()));
    }

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
    {
        if(ec)
            return fail(ec, "connect");

        // Perform the SSL handshake
        stream_.async_handshake(
            ssl::stream_base::client,
            beast::bind_front_handler(
                &session::on_handshake,
                shared_from_this()));
    }

    void on_handshake(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");

        // Set a timeout on the operation
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Send the HTTP request to the remote host
        std::cout << "Sending " << req_ << std::endl;
        http::async_write(stream_, req_,
            beast::bind_front_handler(
                &session::on_write,
                shared_from_this()));
    }

    void on_write(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        // Receive the HTTP response
        http::async_read(stream_, buffer_, res_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }

    void on_read(beast::error_code ec, std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        // Write the message to standard out
        std::cout << res_ << std::endl;

        // Set a timeout on the operation
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Gracefully close the stream
        stream_.async_shutdown(
            beast::bind_front_handler(
                &session::on_shutdown,
                shared_from_this()));
    }

    void on_shutdown(beast::error_code ec)
    {
        if(ec == net::error::eof)
        {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec = {};
        }
        if(ec)
            return fail(ec, "shutdown");

        // If we get here then the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

int main() {
    // The io_context is required for all I/O
    net::io_context ioc;

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12_client};

    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);
    ctx.set_default_verify_paths();

    // Launch the asynchronous operation
    // see for endpoints: https://binance-docs.github.io/apidocs/spot/en/#general-info
    static boost::url_view const base_api{"wss://api.binance.com/api/v3/"};
    boost::url method{"depth"};
    method.params().emplace_back("symbol", "BTCUSDT");

    // The session is constructed with a strand to
    // ensure that handlers do not execute concurrently.
    std::make_shared<session>(net::make_strand(ioc), ctx)
        ->run(make_url(base_api, method));

    // Run the I/O service. The call will return when
    // the get operation is complete.
    ioc.run();
}

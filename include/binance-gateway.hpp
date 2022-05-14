#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/json/src.hpp>
#include <boost/json.hpp>
#include <iostream>
#include <string>


namespace beast = boost::beast;         
namespace http  = beast::http;          
namespace net   = boost::asio;          
namespace ssl   = boost::asio::ssl;     
using tcp       = boost::asio::ip::tcp; 

using executor = net::any_io_executor; 
using namespace boost::json;



boost::url make_url(boost::url_view base_api, boost::url_view method);
void fail_http(beast::error_code ec, char const* what);

namespace binapi{

  namespace AsyncRest{

    struct httpClient : public std::enable_shared_from_this<httpClient> 
    {
        tcp::resolver                        resolver_;
        beast::ssl_stream<beast::tcp_stream> stream_;
        beast::flat_buffer                buffer_; // (Must persist between reads)
        http::request<http::string_body>  req_;
        http::response<http::string_body> res_;
        net::io_context ioc;
        value json;
        

      public:

        httpClient(executor ex, ssl::context& ctx);
        ssl::context ctxx{ssl::context::tlsv12_client};



        // Start the asynchronous operation
        void run(boost::url url, http::verb action);

        void on_resolve(beast::error_code ec, tcp::resolver::results_type results);

        void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);

        void on_handshake(beast::error_code ec);

        void on_write(beast::error_code ec, std::size_t bytes_transferred);

        void on_read(beast::error_code ec, std::size_t bytes_transferred);

        void on_shutdown(beast::error_code ec);
    };
  }
}


namespace binapi
{
    
    namespace AsyncRest
    {

        boost::url make_url(boost::url_view base_api, boost::url_view method) {
            assert(!method.is_path_absolute());
            assert(base_api.data()[base_api.size() - 1] == '/');

            boost::urls::error_code ec;
            boost::url url;
            resolve(base_api, method, url, ec);
            if (ec)
                throw boost::system::system_error(ec);

            std::cout << "URL : "<< url << std::endl;    
            return url;
        }
        // Report a failure
        void fail_http(beast::error_code ec, char const* what)
        {
            std::cerr << what << ": " << ec.message() << "\n";
        }

        httpClient::httpClient(executor ex, ssl::context& ctx)
            : resolver_(ex)
            , stream_(ex, ctx) {}


        // Start the asynchronous operation
        void httpClient::run(boost::url url, http::verb action) 
        {

            std::string const host(url.host());
            std::string const service = url.has_port() //
                ? url.port()
                : (url.scheme_id() == boost::urls::scheme::https) //
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

            // Set up an HTTP GET/POST/DELETE/PUT request message
            // req_.version(version);
            req_.method(action);
            req_.target(url.c_str());
            req_.set(http::field::host, host);
            req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            req_.set("X-MBX-APIKEY", "Acz85W54qihZsUFVL0NtOw5szKysHSpx80c217qBrEYZJhUb15QRJBzMvUp1tFII");
            //req_.body() = serialize(json::object {{"symbol", "btcusdt"}});
            req_.prepare_payload(); // make HTTP 1.1 compliant

            // Look up the domain name

            resolver_.async_resolve(host, service,beast::bind_front_handler(&httpClient::on_resolve,shared_from_this()));

        }

        void httpClient::on_resolve(beast::error_code ec, tcp::resolver::results_type results)
        {
            if(ec)
                return fail_http(ec, "resolve");

            // Set a timeout on the operation
            beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

            // Make the connection on the IP address we get from a lookup
            beast::get_lowest_layer(stream_).async_connect(results,beast::bind_front_handler(&httpClient::on_connect,shared_from_this()));
        }

        void httpClient::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
        {
            if(ec)
                return fail_http(ec, "connect");

            // Perform the SSL handshake
            stream_.async_handshake(ssl::stream_base::client,beast::bind_front_handler(&httpClient::on_handshake,shared_from_this()));
        }

        void httpClient::on_handshake(beast::error_code ec)
        {
            if(ec)
                return fail_http(ec, "handshake");

            // Set a timeout on the operation
            beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

            // Send the HTTP request to the remote host
            std::cout << "Sending " << req_ << std::endl;
            http::async_write(stream_, req_, beast::bind_front_handler(&httpClient::on_write, shared_from_this()));
        }

        void httpClient::on_write(beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            if(ec)
                return fail_http(ec, "write");

            // Receive the HTTP response
            http::async_read(stream_, buffer_, res_, beast::bind_front_handler(&httpClient::on_read,shared_from_this()));

        }

        void httpClient::on_read(beast::error_code ec, std::size_t bytes_transferred)
        {
            boost::ignore_unused(bytes_transferred);

            if(ec)
                return fail_http(ec, "read");

            // Write the message to standard out
            // std::cout << res_.body() << std::endl;

            json = parse(res_.body()).at("serverTime").as_int64();

            std::cout << json <<std::endl;

            // Set a timeout on the operation
            beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

            // Gracefully close the stream
            stream_.async_shutdown(beast::bind_front_handler(&httpClient::on_shutdown,shared_from_this()));
        }

        void httpClient::on_shutdown(beast::error_code ec)
        {
            if(ec == net::error::eof)
            {
                ec = {};
            }
            if(ec)
                return fail_http(ec, "shutdown");

        }

        std::string encryptWithHMAC(const char* key, const char* data) 
        {
            unsigned char *result;
            static char res_hexstring[64];
            int result_len = 32;
            std::string signature;

            result = HMAC(EVP_sha256(), key, strlen((char *)key), const_cast<unsigned char *>(reinterpret_cast<const unsigned char*>(data)), strlen((char *)data), NULL, NULL);
            for (int i = 0; i < result_len; i++) {
                sprintf(&(res_hexstring[i * 2]), "%02x", result[i]);
            }

            for (int i = 0; i < 64; i++) {
                signature += res_hexstring[i];
            }

            return signature;
        }

        static void async_latest_price(std::string symbol, net::io_context &ioc, ssl::context &ctx)
        {
            static boost::url_view const base_api{"https://api.binance.com/api/v3/ticker/"};
            boost::url method{"price"};
            method.params().emplace_back("symbol",symbol);
            std::make_shared<httpClient>(net::make_strand(ioc),ctx)->run(make_url(base_api,method),http::verb::get);

        }


        static void async_exchange_info(std::string symbol, net::io_context &ioc, ssl::context &ctx)
        {

            static boost::url_view const base_api{"https://api.binance.com/api/v3/"};
            boost::url method{"exchangeInfo"};
            method.params().emplace_back("symbol",symbol);
            std::make_shared<httpClient>(net::make_strand(ioc),ctx)->run(make_url(base_api,method),http::verb::get);

        }

        static void async_server_time(net::io_context &ioc, ssl::context &ctx)
        {

            static boost::url_view const base_api{"https://api.binance.com/api/v3/time"};
            std::make_shared<httpClient>(net::make_strand(ioc),ctx)->run(base_api,http::verb::get);

        }


        static void async_ping_binance(net::io_context &ioc, ssl::context &ctx)
        {

            static boost::url_view const base_api{"https://api.binance.com/api/v3/ping"};
            std::make_shared<httpClient>(net::make_strand(ioc),ctx)->run(base_api,http::verb::get);

        }

        static void async_open_orders(net::io_context &ioc, ssl::context &ctx)
        {

            static boost::url_view const base_api{"https://api.binance.com/api/v3/openOrders"};
            std::make_shared<httpClient>(net::make_strand(ioc),ctx)->run(base_api,http::verb::get);

        }

        static void async_orderbook(std::string symbol,std::string levels, net::io_context &ioc, ssl::context &ctx)
        {

            static boost::url_view const base_api{"https://api.binance.com/api/v3/"};
            boost::url method{"depth"};
            method.params().emplace_back("symbol",symbol);
            method.params().emplace_back("limit",levels);
            std::make_shared<httpClient>(net::make_strand(ioc),ctx)->run(make_url(base_api,method),http::verb::get);

        }

        static void async_recent_trades(std::string symbol,std::string levels, net::io_context &ioc, ssl::context &ctx)
        {

            static boost::url_view const base_api{"https://api.binance.com/api/v3/"};
            boost::url method{"trades"};
            method.params().emplace_back("symbol",symbol);
            method.params().emplace_back("limit",levels);
            std::make_shared<httpClient>(net::make_strand(ioc),ctx)->run(make_url(base_api,method),http::verb::get);

        }

        static void async_klines(std::string symbol,std::string interval, net::io_context &ioc, ssl::context &ctx)
        {

            static boost::url_view const base_api{"https://api.binance.com/api/v3/"};
            boost::url method{"klines"};
            method.params().emplace_back("symbol",symbol);
            method.params().emplace_back("interval",interval);
            std::make_shared<httpClient>(net::make_strand(ioc),ctx)->run(make_url(base_api,method),http::verb::get);

        }

        static void async_avg_price(std::string symbol, net::io_context &ioc, ssl::context &ctx)
        {

            static boost::url_view const base_api{"https://api.binance.com/api/v3/"};
            boost::url method{"avgPrice"};
            method.params().emplace_back("symbol",symbol);
            std::make_shared<httpClient>(net::make_strand(ioc),ctx)->run(make_url(base_api,method),http::verb::get);

        }

        static void async_bidask(std::string symbol, net::io_context &ioc, ssl::context &ctx)
        {

            static boost::url_view const base_api{"https://api.binance.com/api/v3/ticker"};
            boost::url method{"bookTicker"};
            method.params().emplace_back("symbol",symbol);
            std::make_shared<httpClient>(net::make_strand(ioc),ctx)->run(make_url(base_api,method),http::verb::get);

        }

    }
}

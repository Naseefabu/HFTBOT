#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <nlohmann/json.hpp>
#include <chrono>
#include <chrono>
#include <iostream>
#include <string>
#include <exception>


namespace beast = boost::beast;         
namespace http  = beast::http;          
namespace net   = boost::asio;          
namespace ssl   = boost::asio::ssl;     
using tcp       = boost::asio::ip::tcp; 

using json = nlohmann::json;

using Clock = std::chrono::system_clock;
using TimePoint = std::chrono::time_point<Clock>;    

using executor = net::any_io_executor; 


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

void fail_http(beast::error_code ec, char const* what);


namespace binapi{

    enum e_side{buy,sell};
    std::string e_side_string(const e_side side);

    enum order_type{limit,market};
    std::string order_type_to_string(order_type type);

    enum timeforce{GTC,IOC,FOK};
    std::string timeforce_to_string(timeforce timeforc);

    enum trade_response_type{ack,result,full,test,unknown};
    std::string trade_response_type_to_string(trade_response_type type);

    namespace rest{

        class httpClient : public std::enable_shared_from_this<httpClient> 
        {

        private:    
            tcp::resolver resolver_;
            beast::ssl_stream<beast::tcp_stream> stream_;
            beast::flat_buffer buffer_; // (Must persist between reads)
            http::request<http::string_body>  req_;
            http::response<http::string_body> res_;
            std::string api_key = "v6uhUtse5Ae1Gyz72eMSbUMGw7VUDdd5AnqobMOW1Llzi4snnfP4YCyY9V74PFJ4";
            std::string secret_key = "FW8j4YobD26PVP6QLu0sv4Dv7OzrtfgQKzn8FoIMwGzMW9Y0VmX1DatbLIfXoCHV";
            std::string base_url = "https://testnet.binance.vision/api/v3/";
            net::io_context& ioc;
            ssl::context& ctx;
            
        public:

            httpClient(executor ex, ssl::context& ctx,net::io_context& ioc);

            http::response<http::string_body> http_call(boost::url url, http::verb action);

            std::string authenticate(const char* key, const char* data);

            json server_time();

            json latest_price(std::string symbol);

            json exchange_info(std::string symbol);

            json get_account_info();

            json check_order_status(std::string symbol,int orderid);

            json cancel_all_orders(std::string symbol);

            json cancel_order(std::string symbol,int orderid);

            json new_order(std::string symbol,int price,e_side side,order_type type,timeforce time,std::string quantity);

            json openOrders();

            json bidask(std::string symbol);

            json avg_price(std::string symbol);

            json klines(std::string symbol,std::string interval);

            json recent_trades(std::string symbol,std::string levels);

            json orderbook(std::string symbol,std::string levels);

            json ping_binance();

        };
    }
}


namespace binapi
{
    std::string e_side_to_string(const e_side side)
    {
        switch (side)
        {
            case buy : return "BUY";
            case sell : return "SELL"; 
        }
        return nullptr;
    }

    std::string order_type_to_string(order_type type)
    {
        switch (type)
        {
            case limit : return "LIMIT";
            case market : return "MARKET";
        }
        return nullptr;
    }
    std::string timeforce_to_string(timeforce timeforc)
    {
        switch (timeforc)
        {
            case GTC : return "GTC";
            case IOC : return "IOC";
            case FOK : return "FOK";
        }
        return nullptr;
    }
    std::string trade_response_type_to_string(trade_response_type type)
    {
        switch(type)
        {
            case ack : return "ACK";
            case result : return "RESULT";
            case full : return "FULL";
            case test : return "TEST";
            case unknown : return "UNKNOWN";
        }
        return nullptr;
    }
    
    namespace rest
    {

        // Report a failure
        void fail_http(beast::error_code ec, char const* what)
        {
            std::cerr << what << ": " << ec.message() << "\n";
        }

        httpClient::httpClient(executor ex, ssl::context& ctxe, net::io_context &ioce)
            : resolver_(ex),stream_(ex, ctxe),ioc(ioce),ctx(ctxe){}


        // Start the asynchronous operation
        http::response<http::string_body> httpClient::http_call(boost::url url, http::verb action) 
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
                // return res_;
            }

            // Set up an HTTP GET/POST/DELETE/PUT request message
            // req_.version(version);
            req_.method(action);
            req_.target(url.c_str());
            req_.set(http::field::host, host);
            req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            req_.set("X-MBX-APIKEY", api_key);
            //req_.body() = serialize(json::object {{"symbol", "btcusdt"}});
            req_.prepare_payload(); // make HTTP 1.1 compliant

            auto const results = resolver_.resolve(host, service);
            beast::get_lowest_layer(stream_).connect(results);

            // Perform the SSL handshake
            stream_.handshake(ssl::stream_base::client);
            http::write(stream_, req_);
            http::read(stream_, buffer_, res_);
            beast::error_code ec;
            stream_.shutdown(ec);

            return res_;

    
        }
        
        json httpClient::server_time()
        {
            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/time"};

            return json::parse(http_call(base_api,http::verb::get).body());
        }

        std::string httpClient::authenticate(const char* key, const char* data) 
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

        json httpClient::latest_price(std::string symbol)
        {
            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/ticker/"};
            boost::url method{"price"};
            method.params().emplace_back("symbol",symbol);
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::get).body());
        }


        json httpClient::exchange_info(std::string symbol)
        {

            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/"};
            boost::url method{"exchangeInfo"};
            method.params().emplace_back("symbol",symbol);
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::get).body());

        }

        json httpClient::ping_binance( )
        {

            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/ping"};
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(base_api,http::verb::get).body());

        }

        json httpClient::orderbook(std::string symbol,std::string levels)
        {
            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/"};
            boost::url method{"depth"};
            method.params().emplace_back("symbol",symbol);
            method.params().emplace_back("limit",levels);
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::get).body());

        }

        json httpClient::recent_trades(std::string symbol,std::string levels)
        {

            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/"};
            boost::url method{"trades"};
            method.params().emplace_back("symbol",symbol);
            method.params().emplace_back("limit",levels);
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::get).body());

        }

        json httpClient::klines(std::string symbol,std::string interval)
        {

            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/"};
            boost::url method{"klines"};
            method.params().emplace_back("symbol",symbol);
            method.params().emplace_back("interval",interval);
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::get).body());

        }

        json httpClient::avg_price(std::string symbol )
        {
            // this->server_time( );
            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/"};
            boost::url method{"avgPrice"};
            method.params().emplace_back("symbol",symbol);
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::get).body());
        }

        json httpClient::bidask(std::string symbol )
        {

            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/ticker/"};
            boost::url method{"bookTicker"};
            method.params().emplace_back("symbol",symbol);
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::get).body());

        }

        json httpClient::openOrders( )
        {
            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/"};
            boost::url method{"openOrders"};

            json time = server_time();
            std::string server_timestamp = time["serverTime"].dump();


            std::string query_params = "timestamp=" + server_timestamp;

            method.params().emplace_back("signature",this->authenticate(this->secret_key.c_str(),query_params.c_str()));  // order matters
            method.params().emplace_back("timestamp",server_timestamp);

            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::get).body());

        }
        
        json httpClient::new_order(std::string symbol,int price,e_side side,order_type type,timeforce time,std::string quantity )
        {
            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/"};
            boost::url method{"order"};
            json times = server_time();
            std::string query_params;
            
            if (type == order_type::market) query_params ="symbol="+symbol+"&side="+e_side_to_string(side) +"&type="+order_type_to_string(type)+ "&quantity="+quantity+"&recvWindow=60000"+"&timestamp=" + times["serverTime"].dump();
            else if(type == order_type::limit)  query_params ="symbol="+symbol+"&side="+e_side_to_string(side) +"&type="+order_type_to_string(type)+ "&timeInForce="+timeforce_to_string(time)+ "&quantity="+quantity+"&price="+std::to_string(price)+"&recvWindow=60000"+"&timestamp=" + times["serverTime"].dump();

            method.params().emplace_back("symbol",symbol);
            method.params().emplace_back("side",e_side_to_string(side));
            method.params().emplace_back("type",order_type_to_string(type));
            if (type == order_type::limit) method.params().emplace_back("timeInForce",timeforce_to_string(time));
            method.params().emplace_back("quantity",quantity); 
            if (type == order_type::limit) method.params().emplace_back("price",std::to_string(price));
            method.params().emplace_back("recvWindow", "60000");
            method.params().emplace_back("timestamp",times["serverTime"].dump());
            method.params().emplace_back("signature",this->authenticate(this->secret_key.c_str(),query_params.c_str())); 
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::post).body());

        }

        json httpClient::cancel_order(std::string symbol,int orderid )
        {
            json time = server_time();
            std::string query_params = "symbol="+symbol+"&orderId="+std::to_string(orderid)+"&timestamp="+time["serverTime"].dump();

            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/"};
            boost::url method{"order"};
            method.params().emplace_back("symbol",symbol);
            method.params().emplace_back("orderId",std::to_string(orderid));
            method.params().emplace_back("signature",this->authenticate(this->secret_key.c_str(),query_params.c_str()));
            method.params().emplace_back("timestamp",time["serverTime"].dump());
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::delete_).body());

        }

        json httpClient::cancel_all_orders(std::string symbol )
        {
            json time = server_time();
            std::string query_params = "symbol="+symbol+"&timestamp="+time["serverTime"].dump();
            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/"};
            boost::url method{"openOrders"};
            method.params().emplace_back("symbol",symbol);
            method.params().emplace_back("signature",this->authenticate(this->secret_key.c_str(),query_params.c_str()));
            method.params().emplace_back("timestamp",time["serverTime"].dump());
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::delete_).body());

        }
        
        json httpClient::check_order_status(std::string symbol,int orderid )
        {
            json time = server_time();
            std::string query_params = "symbol="+symbol+"&orderId="+std::to_string(orderid)+"&timestamp="+time["serverTime"].dump();

            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/"};
            boost::url method{"order"};
            method.params().emplace_back("symbol",symbol);
            method.params().emplace_back("orderId",std::to_string(orderid));
            method.params().emplace_back("signature",this->authenticate(this->secret_key.c_str(),query_params.c_str()));
            method.params().emplace_back("timestamp",time["serverTime"].dump());
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::get).body());

        }

        json httpClient::get_account_info()
        {
            json time = server_time();
            std::string query_params = "timestamp="+time["serverTime"].dump();

            static boost::url_view const base_api{"https://testnet.binance.vision/api/v3/"};
            boost::url method{"account"};
            method.params().emplace_back("signature",this->authenticate(this->secret_key.c_str(),query_params.c_str()));
            method.params().emplace_back("timestamp",time["serverTime"].dump());
            return json::parse(std::make_shared<httpClient>(ioc.get_executor(),ctx,ioc)->http_call(make_url(base_api,method),http::verb::get).body());

        }



    }
}



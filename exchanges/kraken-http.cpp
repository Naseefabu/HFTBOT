#include "kraken-http.hpp"

#include "coinbase-http.hpp"


KrakenAPI::KrakenAPI(executor ex, ssl::context& ctxe, net::io_context &ioce)
    : resolver_(ex),stream_(ex, ctxe),ioc(ioce),ctx(ctxe){
        PostDecodeString = decode64(this->secret_key);
    }


http::response<http::string_body> KrakenAPI::http_call(boost::url url, http::verb action) 
{

    std::string const host(url.host());

    std::string const service = "https";
    url.remove_origin(); 

    SSL_set_tlsext_host_name(stream_.native_handle(), host.c_str());

    req_.method(action);
    req_.target(url.c_str());
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req_.set("Content-Type","application/json");
    req_.prepare_payload();

    auto const results = resolver_.resolve(host, service);
    beast::get_lowest_layer(stream_).connect(results);

    stream_.handshake(ssl::stream_base::client);

    http::write(stream_, req_);
    http::read(stream_, buffer_, res_);
    beast::error_code ec;
    stream_.shutdown(ec);

    return res_;
}

void KrakenAPI::configure(const std::string &api, const std::string &secret)
{
    this->api_key = api;
    this->secret_key = secret;
}

// interval in minutes
json KrakenAPI::GET_OHLC(std::string pair, std::string interval)
{
    boost::url method{"public/OHLC"};
    method.params().emplace_back("pair",pair);
    method.params().emplace_back("interval", interval);
    return json::parse(http_call(make_url(base_api,method),http::verb::get).body());
}

json KrakenAPI::get_orderbook(std::string pair, std::string levels)
{
    boost::url method{"public/Depth"};
    method.params().emplace_back("pair",pair);
    method.params().emplace_back("count", levels);
    return json::parse(http_call(make_url(base_api,method),http::verb::get).body());
}

json KrakenAPI::get_trades(std::string pair, std::string sinceTime)
{
    boost::url method{"public/Depth"};
    method.params().emplace_back("pair",pair);
    method.params().emplace_back("since", sinceTime);
    return json::parse(http_call(make_url(base_api,method),http::verb::get).body());
}


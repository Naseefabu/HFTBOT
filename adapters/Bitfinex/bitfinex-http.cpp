#include "bitfinex-http.hpp"

bitfinexAPI::bitfinexAPI(executor ex, ssl::context& ctxe, net::io_context &ioce)
    : resolver_(ex),stream_(ex, ctxe),ioc(ioce),ctx(ctxe)
    {
        PostDecodeString = decode64(this->secret_key);
    }


http::response<http::string_body> bitfinexAPI::http_call(boost::url url, http::verb action) 
{

    std::string const host(url.host());
    std::string const service = "https";
    url.remove_origin(); 

    SSL_set_tlsext_host_name(stream_.native_handle(), host.c_str());

    req_.method(action);
    req_.target(url.c_str());
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req_.set("bfx-apikey", api_key);
    req_.set("Content-Type", "application/json");
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

// valid levels : 1,25,100
json bitfinexAPI::get_snapshot(const std::string &symbol,const std::string &levels)
{
    std::string path_params = "book/"+symbol+"/P3";

    boost::url method{path_params};
    method.params().emplace_back("len",levels);
    return json::parse(std::make_shared<bitfinexAPI>(ioc.get_executor(),ctx,ioc)->http_call(make_url(public_base_api,method),http::verb::get).body());

}

json bitfinexAPI::place_market_buy(std::string symbol,std::string amount)
{
    boost::url method{ "auth/w/order/submit"};

    nlohmann::ordered_json payload = {{"type", "MARKET"},
        {"symbol", symbol},
        {"amount",amount}};

    long NONCE = generate_nonce();
    std::string message = "/api/v2/auth/w/order/submit" + std::to_string(NONCE) + payload.dump();
    std::string sign = getHmacSha384(message,secret_key);
    req_.target("/v2/auth/w/order/submit");
    req_.set("bfx-nonce",std::to_string(NONCE));
    req_.set("bfx-signature",sign);
    req_.body() = payload.dump();
    return json::parse(http_call(make_url(private_base_api,method),http::verb::post).body());
    
}

json bitfinexAPI::place_market_sell(std::string symbol,std::string amount)
{

    boost::url method{ "auth/w/order/submit"};
    amount.insert(0,"-");
    nlohmann::ordered_json payload = {{"type", "MARKET"},
        {"symbol", symbol},
        {"amount",amount}};

    long NONCE = generate_nonce();
    std::string message = "/api/v2/auth/w/order/submit" + std::to_string(NONCE) + payload.dump();
    std::string sign = getHmacSha384(message,secret_key);
    req_.target("/v2/auth/w/order/submit");
    req_.set("bfx-nonce",std::to_string(NONCE));
    req_.set("bfx-signature",sign);
    req_.body() = payload.dump();
    return json::parse(http_call(make_url(private_base_api,method),http::verb::post).body());
    
}

json bitfinexAPI::cancel_order(int orderid)
{

    boost::url method{ "auth/w/order/cancel"};
    nlohmann::ordered_json payload = {{"id", orderid}};

    long NONCE = generate_nonce();
    std::string message = "/api/v2/auth/w/order/cancel" + std::to_string(NONCE) + payload.dump();
    std::string sign = getHmacSha384(message,secret_key);
    req_.set("bfx-nonce",std::to_string(NONCE));
    req_.set("bfx-signature",sign);
    req_.body() = payload.dump();
    return json::parse(http_call(make_url(private_base_api,method),http::verb::post).body());
    
}

json bitfinexAPI::cancel_all_orders()
{

    boost::url method{ "auth/w/order/cancel/multi"};

    long NONCE = generate_nonce();
    std::string message = "/api/v2/auth/w/order/cancel/multi" + std::to_string(NONCE);
    std::string sign = getHmacSha384(message,secret_key);
    req_.set("bfx-nonce",std::to_string(NONCE));
    req_.set("bfx-signature",sign);
    return json::parse(http_call(make_url(private_base_api,method),http::verb::post).body());
    
}
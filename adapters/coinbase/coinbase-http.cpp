#include "coinbase-http.hpp"


coinbaseAPI::coinbaseAPI(executor ex, ssl::context& ctxe, net::io_context &ioce)
    : resolver_(ex),stream_(ex, ctxe),ioc(ioce),ctx(ctxe){
        PostDecodeString = decode64(this->secret_key);
    }


http::response<http::string_body> coinbaseAPI::http_call(boost::url url, http::verb action) 
{

    std::string const host(url.host());

    std::string const service = "https";
    url.remove_origin(); 

    SSL_set_tlsext_host_name(stream_.native_handle(), host.c_str());

    req_.method(action);
    req_.target(url.c_str());
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req_.set("CB-ACCESS-KEY",api_key);
    req_.set("CB-ACCESS-PASSPHRASE",passphrase);
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

std::string coinbaseAPI::authenticate(const char* key, const char* data) 
{
    unsigned char *result;
    static char res_hexstring[64];
    int result_len = 32;
    std::string signature;

    result = HMAC(EVP_sha256(), key, strlen((char *)key), const_cast<unsigned char *>(reinterpret_cast<const unsigned char*>(data)), strlen((char *)data), NULL, NULL);
    const char *preEncodeSignature_c = strdup(reinterpret_cast<const char *>(result));
    std::string preEncodeSignature(preEncodeSignature_c);

    std::string postEncodeSignature = encode64(preEncodeSignature);

    return postEncodeSignature;
}

void coinbaseAPI::configure(const std::string &api, const std::string &secret,const std::string &pass)
{
    this->api_key = api;
    this->secret_key = secret;
    this->passphrase = pass;
}

json coinbaseAPI::open_orders()
{
    boost::url method{"orders"};
    std::string time = std::to_string(get_sec_timestamp(current_time()).count());
    std::string data = time + "GET" + "/orders";
    sign = authenticate(PostDecodeString.c_str(),data.c_str());
    req_.set("CB-ACCESS-SIGN",sign);
    req_.set("CB-ACCESS-TIMESTAMP", time);
    return json::parse(http_call(make_url(base_api,method),http::verb::get).body());
}

json coinbaseAPI::cancel_all()
{
    boost::url method{"orders"};
    std::string time = std::to_string(get_sec_timestamp(current_time()).count());
    std::string data = time + "DELETE" + "/orders";
    sign = authenticate(PostDecodeString.c_str(),data.c_str());
    req_.set("CB-ACCESS-SIGN",sign);
    req_.set("CB-ACCESS-TIMESTAMP", time);
    return json::parse(http_call(make_url(base_api,method),http::verb::delete_).body());
}

// "Trading Pair is not allowed" => most likely because of your region that is permission denied
ordered_json coinbaseAPI::place_market_buy(std::string market,std::string size)
{
    boost::url method{"orders"};
    std::string time = std::to_string(get_sec_timestamp(current_time()).count());

    ordered_json payload = {{"type", "market"},
            {"side", "buy"},
            {"product_id",market},
            {"funds", size}};

    std::string data = time + "POST" + "/orders" + payload.dump();
    sign = authenticate(PostDecodeString.c_str(),data.c_str());
    req_.set("CB-ACCESS-SIGN",sign);
    req_.set("CB-ACCESS-TIMESTAMP", time);
    req_.body() = payload.dump();
    return json::parse(http_call(make_url(base_api,method),http::verb::post).body());
}

ordered_json coinbaseAPI::place_market_sell(std::string market,std::string size)
{
    boost::url method{"orders"};
    std::string time = std::to_string(get_sec_timestamp(current_time()).count());

    ordered_json payload = {{"type", "market"},
            {"side", "sell"},
            {"product_id",market},
            {"size", size}};

    std::string data = time + "POST" + "/orders" + payload.dump();
    sign = authenticate(PostDecodeString.c_str(),data.c_str());
    req_.set("CB-ACCESS-SIGN",sign);
    req_.set("CB-ACCESS-TIMESTAMP", time);
    req_.body() = payload.dump();
    return json::parse(http_call(make_url(base_api,method),http::verb::post).body());
}

ordered_json coinbaseAPI::place_limit_order(std::string market,int price,std::string size, std::string side)
{
    boost::url method{"orders"};
    std::string time = std::to_string(get_sec_timestamp(current_time()).count());

    ordered_json payload = {{"type", "limit"},
            {"side", side},
            {"product_id",market},
            {"price",std::to_string(price)},
            {"size", size},
            {"post_only",true}};

    std::string data = time + "POST" + "/orders" + payload.dump();
    sign = authenticate(PostDecodeString.c_str(),data.c_str());
    req_.set("CB-ACCESS-SIGN",sign);
    req_.set("CB-ACCESS-TIMESTAMP", time);
    req_.body() = payload.dump();
    return json::parse(http_call(make_url(base_api,method),http::verb::post).body());
}

json coinbaseAPI::cancel_order(int orderid)
{
    boost::url method{"orders/"+std::to_string(orderid)};
    std::string time = std::to_string(get_sec_timestamp(current_time()).count());
    std::string data = time + "DELETE" + "/orders/"+std::to_string(orderid);
    sign = authenticate(PostDecodeString.c_str(),data.c_str());
    req_.set("CB-ACCESS-SIGN",sign);
    req_.set("CB-ACCESS-TIMESTAMP", time);
    return json::parse(http_call(make_url(base_api,method),http::verb::delete_).body());
}

json coinbaseAPI::get_snapshot(std::string market, int level)
{
    boost::url method{"products/"+market+"/"+"book"};
    method.params().emplace_back("level",std::to_string(level));
    return json::parse(http_call(make_url(base_api,method),http::verb::get).body());
}
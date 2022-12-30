#include "kraken-http.hpp"

#include "coinbase-http.hpp"

KrakenAPI::KrakenAPI(executor ex, ssl::context &ctxe, net::io_context &ioce)
    : resolver_(ex), stream_(ex, ctxe), ioc(ioce), ctx(ctxe)
{
    PostDecodeString = decode64(this->secret_key);
}

http::response<http::string_body> KrakenAPI::http_call(boost::url url, http::verb action)
{

    std::string const host(url.host());

    std::string const service = "https";
    url.remove_origin();
    url.remove_origin();

    SSL_set_tlsext_host_name(stream_.native_handle(), host.c_str());

    req_.method(action);
    req_.target(url.c_str());
    req_.set(http::field::host, host);
    req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req_.set("API-Key", api_key);
    req_.set("Content-Type", "application/x-www-form-urlencoded");
    req_.prepare_payload();
    std::cout << "raw request : " << req_ << std::endl;

    auto const results = resolver_.resolve(host, service);
    beast::get_lowest_layer(stream_).connect(results);

    stream_.handshake(ssl::stream_base::client);

    http::write(stream_, req_);
    http::read(stream_, buffer_, res_);
    std::cout << "response : " << res_.body() << std::endl;
    std::cout << "raw response : " << res_ << std::endl;

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
json KrakenAPI::get_ohlc(std::string pair, std::string interval)
{
    boost::url method{"public/OHLC"};
    method.params().emplace_back("pair", pair);
    method.params().emplace_back("interval", interval);
    return json::parse(http_call(make_url(base_api, method), http::verb::get).body());
}

json KrakenAPI::get_orderbook(std::string pair, std::string levels)
{
    boost::url method{"public/Depth"};
    method.params().emplace_back("pair", pair);
    method.params().emplace_back("count", levels);
    return json::parse(http_call(make_url(base_api, method), http::verb::get).body());
}

json KrakenAPI::get_trades(std::string pair, std::string sinceTime)
{
    boost::url method{"public/Depth"};
    method.params().emplace_back("pair", pair);
    method.params().emplace_back("since", sinceTime);
    return json::parse(http_call(make_url(base_api, method), http::verb::get).body());
}

json KrakenAPI::submit_market_order(std::string action, std::string pair, double size)
{
    boost::url method{"private/AddOrder"};
    long nonce = generate_nonce();
    std::string postdata1 = "nonce=" + std::to_string(nonce) + "&ordertype=market&type=" + action +
                            "&volume=" + std::to_string(size) + "&pair=" + pair;
    std::string encoded_data = encode_url(postdata1);
    std::string signature = krak_signature("/0/private/AddOrder", std::to_string(nonce), postdata1, secret_key);
    req_.set("API-Sign", signature);
    req_.body() = postdata1;
    boost::url url = make_url(base_api, method);
    return json::parse(http_call(make_url(base_api, method), http::verb::post).body());
}

json KrakenAPI::submit_limit_order(std::string action, std::string pair, double size, double price)
{
    boost::url method{"private/AddOrder"};
    long nonce = generate_nonce();
    std::string postdata1 = "nonce=" + std::to_string(nonce) + "&ordertype=limit&type=" + action +
                            "&volume=" + std::to_string(size) + "&pair=" + pair + "&price=" + std::to_string(price);
    std::string encoded_data = encode_url(postdata1);
    std::string signature = krak_signature("/0/private/AddOrder", std::to_string(nonce), postdata1, secret_key);
    req_.set("API-Sign", signature);
    req_.body() = postdata1;
    boost::url url = make_url(base_api, method);
    return json::parse(http_call(make_url(base_api, method), http::verb::post).body());
}

json KrakenAPI::cancel_order(int id)
{
    boost::url method{"private/CancelOrder"};
    long nonce = generate_nonce();
    std::string postdata1 = "nonce=" + std::to_string(nonce) + "&txid=" + std::to_string(id);
    std::string encoded_data = encode_url(postdata1);
    std::string signature = krak_signature("/0/private/CancelOrder", std::to_string(nonce), postdata1, secret_key);
    req_.set("API-Sign", signature);
    req_.body() = postdata1;
    boost::url url = make_url(base_api, method);
    return json::parse(http_call(make_url(base_api, method), http::verb::post).body());
}

json KrakenAPI::cancel_all_orders()
{
    boost::url method{"private/CancelOrder"};
    long nonce = generate_nonce();
    std::string postdata1 = "nonce=" + std::to_string(nonce);
    std::string encoded_data = encode_url(postdata1);
    std::string signature = krak_signature("/0/private/CancelAll", std::to_string(nonce), postdata1, secret_key);
    req_.set("API-Sign", signature);
    req_.body() = postdata1;
    boost::url url = make_url(base_api, method);
    return json::parse(http_call(make_url(base_api, method), http::verb::post).body());
}

json KrakenAPI::get_account_balance()
{
    boost::url method{"private/Balance"};
    long nonce = generate_nonce();
    std::string postdata1 = "nonce=" + std::to_string(nonce);
    std::string encoded_data = encode_url(postdata1);
    std::string signature = krak_signature("/0/private/Balance", std::to_string(nonce), postdata1, secret_key);
    req_.set("API-Sign", signature);
    req_.body() = postdata1;
    boost::url url = make_url(base_api, method);
    return json::parse(http_call(make_url(base_api, method), http::verb::post).body());
}

json KrakenAPI::get_open_orders()
{
    boost::url method{"private/OpenOrders"};
    long nonce = generate_nonce();
    std::string postdata1 = "nonce=" + std::to_string(nonce);
    std::string encoded_data = encode_url(postdata1);
    std::string signature = krak_signature("/0/private/OpenOrders", std::to_string(nonce), postdata1, secret_key);
    req_.set("API-Sign", signature);
    req_.body() = postdata1;
    boost::url url = make_url(base_api, method);
    return json::parse(http_call(make_url(base_api, method), http::verb::post).body());
}
#include "bitfinex-http.hpp"

bitfinexAPI::bitfinexAPI(executor ex, ssl::context& ctxe, net::io_context &ioce)
    : resolver_(ex),stream_(ex, ctxe),ioc(ioce),ctx(ctxe){}


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
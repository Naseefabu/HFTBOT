#include "httpClient.hpp"

int main()
{
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

    // query paramters
    method.params().emplace_back("symbol", "BTCUSDT");

    // The session is constructed with a strand to
    // ensure that handlers do not execute concurrently.
    std::make_shared<httpClient>(net::make_strand(ioc), ctx)
        ->run(make_url(base_api, method));

    // Run the I/O service. The call will return when
    // the get operation is complete.
    ioc.run();
}
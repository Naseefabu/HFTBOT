#pragma once

#include <iostream>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/url.hpp>
namespace beast = boost::beast;   

using namespace std::chrono;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
using Clock = std::chrono::system_clock;
using TimePoint = std::chrono::time_point<Clock>;




TimePoint current_time();

std::chrono::milliseconds get_ms_timestamp(TimePoint time);

boost::url make_url(boost::url base_api, boost::url method);

void fail_http(beast::error_code ec, char const* what);

void fail_ws(beast::error_code ec, char const* what);




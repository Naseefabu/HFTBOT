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


enum e_side{buy,sell};
std::string e_side_to_string(const e_side side);

enum order_type{limit,market};
std::string order_type_to_string(order_type type);

enum timeforce{GTC,IOC,FOK};
std::string timeforce_to_string(timeforce timeforc);

enum trade_response_type{ack,result,full,test,unknown};
std::string trade_response_type_to_string(trade_response_type type);

boost::url make_url(boost::url base_api, boost::url method);

void fail_http(beast::error_code ec, char const* what);

void fail_ws(beast::error_code ec, char const* what);




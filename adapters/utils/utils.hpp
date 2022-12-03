#pragma once

#include <iostream>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/crc.hpp> 
#include <boost/url.hpp>
#include <stdint.h>
#include <string>
namespace beast = boost::beast;   

using namespace std::chrono;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;
using Clock = std::chrono::system_clock;
using TimePoint = std::chrono::time_point<Clock>;

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/insert_linebreaks.hpp>
#include <boost/archive/iterators/remove_whitespace.hpp>
#include <boost/algorithm/string.hpp>

using namespace boost::archive::iterators;




TimePoint current_time();

std::chrono::milliseconds get_ms_timestamp(TimePoint time);

std::chrono::seconds get_sec_timestamp(TimePoint time);

boost::url make_url(boost::url base_api, boost::url method);

void fail_http(beast::error_code ec, char const* what);

void fail_ws(beast::error_code ec, char const* what);

std::string encode64(std::string &val);

std::string decode64(const std::string &val);

std::string removeDecimalAndLeadingZeros(std::string str);

uint32_t checksum32(const std::string &str);





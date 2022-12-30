#pragma once

#include <iostream>
#include <chrono>
#include <sstream>
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

std::string getHmacSha256(const char* key, const char* data);

std::string coinbase_HmacSha256(const char* key, const char* data);

long generate_nonce();

std::string getHmacSha384(std::string &key,std::string &content);

std::string encode_url(std::string input);


#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>

std::vector<unsigned char> hmac_sha512(const std::vector<unsigned char>& data, const std::vector<unsigned char>& key);
std::string b64_encode(const std::vector<unsigned char>& data); 
std::vector<unsigned char> b64_decode(const std::string& data); 
std::vector<unsigned char> sha256(const std::string& data);
std::string krak_signature(std::string path, std::string nonce, std::string postdata, std::string secret);

#include "utils.hpp"



TimePoint current_time()
{
    return Clock::now();
}

std::chrono::milliseconds get_ms_timestamp(TimePoint time)
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch());
}

std::chrono::seconds get_sec_timestamp(TimePoint time)
{
    return std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch());
}

void fail_http(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

boost::url make_url(boost::url base_api, boost::url method){
    assert(!method.is_path_absolute());
    assert(base_api.data()[base_api.size() - 1] == '/');

    boost::urls::error_code ec;
    boost::url url;
    resolve(base_api, method, url, ec);
    if (ec)
        throw boost::system::system_error(ec);
    return url;
}

void fail_ws(beast::error_code ec, char const* what)
{
    std::cerr << what << ": " << ec.message() << "\n";
}

std::string encode64(std::string &val) {
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
    auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
    return tmp.append((3 - val.size() % 3) % 3, '=');
}

std::string decode64(const std::string &val) {
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), [](char c) {
        return c == '\0';
    });
}


std::string removeDecimalAndLeadingZeros(std::string str) {
  // Remove the decimal point
  str.erase(std::remove(str.begin(), str.end(), '.'), str.end());

  // Remove leading zeros
  while (str.length() > 1 && str[0] == '0') {
    str.erase(0, 1);
  }

  return str;
}

// there is compiler instrinsics for checksum32 calculation as part of SSE4 instruction set, will refer later for more optimizations
uint32_t checksum32(const std::string &str){
    boost::crc_32_type result;
    result.process_bytes(str.data(), str.length());

    // Cast the checksum to an unsigned 32-bit integer
    uint32_t checksum = result.checksum();

    return checksum;
}


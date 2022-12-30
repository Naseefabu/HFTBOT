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

std::string coinbase_HmacSha256(const char* key, const char* data) 
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

std::string getHmacSha256(const char* key, const char* data) 
{
    unsigned char *result;
    static char res_hexstring[64];
    int result_len = 32;
    std::string signature;

    result = HMAC(EVP_sha256(), key, strlen((char *)key), const_cast<unsigned char *>(reinterpret_cast<const unsigned char*>(data)), strlen((char *)data), NULL, NULL);
    for (int i = 0; i < result_len; i++) {
        sprintf(&(res_hexstring[i * 2]), "%02x", result[i]);
    }

    for (int i = 0; i < 64; i++) {
        signature += res_hexstring[i];
    }

    return signature;
}

std::string getHmacSha384(std::string &key, std::string &content)
{
    unsigned char *result;
    int result_len = 48;
    std::string digest;
    std::string hex_digest;

    result = HMAC(EVP_sha384(), key.data(), key.size(),
                  reinterpret_cast<const unsigned char *>(content.data()),
                  content.size(), NULL, NULL);

    // Generate a string of hexadecimal digits from the MAC
    std::stringstream ss;
    for (int i = 0; i < result_len; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)result[i];
    }
    hex_digest = ss.str();

    return hex_digest;
}

std::string encode_url(std::string input) {
  std::stringstream encoded;
  for (const char& c : input) {
    if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      encoded << c;
    } else {
      encoded << '%' << std::setw(2) << std::setfill('0') << std::uppercase << std::hex << static_cast<int>(c);
    }
  }
  return encoded.str();
}

long generate_nonce() {
    return std::chrono::system_clock::now().time_since_epoch() / 1ms;
}



// temp test

std::string krak_signature(std::string path, std::string nonce, const std::string postdata, std::string secret) 
{
   // add path to data to encrypt
   std::vector<unsigned char> data(path.begin(), path.end());

   // concatenate nonce and postdata and compute SHA256
   std::vector<unsigned char> nonce_postdata = sha256(nonce + postdata);

   // concatenate path and nonce_postdata (path + sha256(nonce + postdata))
   data.insert(data.end(), nonce_postdata.begin(), nonce_postdata.end());

   // and compute HMAC
   return b64_encode( hmac_sha512(data, b64_decode(secret)) );
}

std::vector<unsigned char> sha256(const std::string& data)
{
   std::vector<unsigned char> digest(SHA256_DIGEST_LENGTH);

   SHA256_CTX ctx;
   SHA256_Init(&ctx);
   SHA256_Update(&ctx, data.c_str(), data.length());
   SHA256_Final(digest.data(), &ctx);

   return digest;
}

std::vector<unsigned char> b64_decode(const std::string& data) 
{
   BIO* b64 = BIO_new(BIO_f_base64());
   BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

   BIO* bmem = BIO_new_mem_buf((void*)data.c_str(),data.length());
   bmem = BIO_push(b64, bmem);
   
   std::vector<unsigned char> output(data.length());
   int decoded_size = BIO_read(bmem, output.data(), output.size());
   BIO_free_all(bmem);

   if (decoded_size < 0)
      throw std::runtime_error("failed while decoding base64.");
   
   return output;
}

std::string b64_encode(const std::vector<unsigned char>& data) 
{
   BIO* b64 = BIO_new(BIO_f_base64());
   BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

   BIO* bmem = BIO_new(BIO_s_mem());
   b64 = BIO_push(b64, bmem);
   
   BIO_write(b64, data.data(), data.size());
   BIO_flush(b64);

   BUF_MEM* bptr = NULL;
   BIO_get_mem_ptr(b64, &bptr);
   
   std::string output(bptr->data, bptr->length);
   BIO_free_all(b64);

   return output;
}

std::vector<unsigned char> hmac_sha512(const std::vector<unsigned char>& data, const std::vector<unsigned char>& key)
{   
   unsigned int len = EVP_MAX_MD_SIZE;
   std::vector<unsigned char> digest(len);

   HMAC_CTX *ctx = HMAC_CTX_new();
   if (ctx == NULL) {
       throw std::runtime_error("cannot create HMAC_CTX");
   }

   HMAC_Init_ex(ctx, key.data(), key.size(), EVP_sha512(), NULL);
   HMAC_Update(ctx, data.data(), data.size());
   HMAC_Final(ctx, digest.data(), &len);
   
   HMAC_CTX_free(ctx);
   
   return digest;
}
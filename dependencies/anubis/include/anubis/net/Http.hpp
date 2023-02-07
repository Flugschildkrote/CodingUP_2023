#ifndef ANUBIS_NET_HTTP_HPP
#define ANUBIS_NET_HTTP_HPP

/* compilation: gcc -o client client.c -lssl -lcrypto */

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <openssl/bio.h> /* BasicInput/Output streams */
#include <openssl/err.h> /* errors */
#include <openssl/ssl.h> /* core library */
#include <openssl/sha.h>
#include <curl/curl.h>
#include <json/json.h>
#include <functional>
#include <sstream>

namespace anubis {
namespace net {

class HTTPSession 
{
public:
    template <typename T>
    using Reader = std::function<size_t(std::string_view, T&)>;

    template <typename T>
    using Writer = std::function<std::string(const T&)>;

    HTTPSession(const std::string& server, bool secure = false)
        : m_Session(curl_easy_init()),
          m_Server((secure ? "https://" : "http://") + server)
    {

    }
    ~HTTPSession(void) { curl_easy_cleanup(m_Session); }

    template <typename T_RESULT, typename T_DATA, typename = std::enable_if_t<std::negation_v<std::is_void<T_RESULT>>>>
    CURLcode post(const std::string& endpoint, const T_DATA& data, Writer<T_DATA> writer, T_RESULT& out_Result, Reader<T_RESULT> reader, const std::string& contentType = "text/plain") {

        struct UserData {
            T_RESULT& outData;
            Reader<T_RESULT>& reader;
        };

        auto read_callback = [](void* inRawData, size_t always1, size_t inDataSize, void* userDataRaw) -> size_t {
            const char* strStart = reinterpret_cast<const char*>(inRawData);
            const char* strEnd = strStart + inDataSize;

            std::string_view inStr(strStart, strEnd);
            UserData* userData = reinterpret_cast<UserData*>(userDataRaw);

            return userData->reader(inStr, userData->outData);
        };
        UserData readData = { .outData = out_Result, .reader = reader };
       
        return postImpl(endpoint, data, writer, contentType, read_callback, &readData);

    }

    template <typename T_RESULT, typename T_DATA, typename=std::enable_if_t<std::is_void_v<T_RESULT>>>
    CURLcode post(const std::string& endpoint, const T_DATA& data, Writer<T_DATA> writer, const std::string& contentType = "text/plain") {

        auto read_callback = [](void* inRawData, size_t always1, size_t inDataSize, void* userDataRaw) -> size_t {
            return inDataSize;
        };

        return postImpl(endpoint, data, writer, contentType, read_callback, nullptr);
    }

    template <typename T_RESULT>
    CURLcode get(const std::string& endpoint, T_RESULT& outData, Reader<T_RESULT> reader)
    {
        struct UserData {
            T_RESULT& outData;
            Reader<T_RESULT>& reader;
        };

        auto read_callback = [](void* inRawData, size_t always1, size_t inDataSize, void* userDataRaw) -> size_t {
            const char* strStart = reinterpret_cast<const char*>(inRawData);
            const char* strEnd = strStart + inDataSize;

            std::string_view inStr(strStart, strEnd);
            UserData* userData = reinterpret_cast<UserData*>(userDataRaw);

            return userData->reader(inStr, userData->outData);
        };
        UserData readData = { .outData = outData, .reader = reader };

        return getImpl(endpoint, read_callback, &readData);
    }

private:

    /*
     * @param ptr Received data from the server 
     * @param size Always 1
     * @param nmemb Size of the received data 
     */

    using CurlWriteFnc_t = size_t(*)(void* inRawData, size_t always1, size_t inDataSize, void* userDataRaw);

    CURLcode getImpl(const std::string& endpoint, CurlWriteFnc_t writeCallback, void* writeData) {
        std::string url = m_Server + endpoint;

        curl_easy_setopt(m_Session, CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_Session, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(m_Session, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(m_Session, CURLOPT_COOKIEFILE, "");
        curl_easy_setopt(m_Session, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(m_Session, CURLOPT_WRITEDATA, writeData);
        curl_easy_setopt(m_Session, CURLOPT_HTTPGET, 1L);

        CURLcode curlStatus = curl_easy_perform(m_Session);
        return curlStatus;
    }

    template <typename T_DATA>
    CURLcode postImpl(const std::string& endpoint, const T_DATA& data, Writer<T_DATA>& writer, const std::string& contentType, CurlWriteFnc_t writeCallback, void* writeData) {
        std::string url = m_Server + endpoint;
        std::string postData = writer(data);

        std::string contentTypeHeader = "Content-Type: " + contentType;
        curl_slist* headerList = curl_slist_append(nullptr, contentTypeHeader.c_str());

        curl_easy_setopt(m_Session, CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_Session, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(m_Session, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(m_Session, CURLOPT_COOKIEFILE, "");
        curl_easy_setopt(m_Session, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(m_Session, CURLOPT_WRITEDATA, writeData);
        curl_easy_setopt(m_Session, CURLOPT_POSTFIELDS, postData.c_str());
        curl_easy_setopt(m_Session, CURLOPT_HTTPHEADER, headerList);

        CURLcode curlStatus = curl_easy_perform(m_Session);
        curl_slist_free_all(headerList);
        return curlStatus;
    }

    std::string m_Server;
    CURL* m_Session;

};

}
}

#endif // !ANUBIS_NET_HTTP_HPP
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

#define ANUBIS_NET_ENABLE_DUMP 1

namespace anubis {
namespace net {

#if ANUBIS_NET_ENABLE_DUMP != 0
    static void dump(const char* text,
            FILE* stream, unsigned char* ptr, size_t size,
            char nohex)
    {
        size_t i;
        size_t c;

        unsigned int width = 0x10;

        if (nohex)
            // without the hex output, we can fit more on screen 
            width = 0x40;

        fprintf(stream, "%s, %10.10lu bytes (0x%8.8lx)\n",
            text, (unsigned long)size, (unsigned long)size);

        for (i = 0; i < size; i += width) {

            fprintf(stream, "%4.4lx: ", (unsigned long)i);

            if (!nohex) {
                // hex not disabled, show it 
                for (c = 0; c < width; c++)
                    if (i + c < size)
                        fprintf(stream, "%02x ", ptr[i + c]);
                    else
                        fputs("   ", stream);
            }

            for (c = 0; (c < width) && (i + c < size); c++) {
                // check for 0D0A; if found, skip past and start a new line of output 
                if (nohex && (i + c + 1 < size) && ptr[i + c] == 0x0D &&
                    ptr[i + c + 1] == 0x0A) {
                    i += (c + 2 - width);
                    break;
                }
                fprintf(stream, "%c",
                    (ptr[i + c] >= 0x20) && (ptr[i + c] < 0x80) ? ptr[i + c] : '.');
                // check again for 0D0A, to avoid an extra \n if it's at width
                if (nohex && (i + c + 2 < size) && ptr[i + c + 1] == 0x0D &&
                    ptr[i + c + 2] == 0x0A) {
                    i += (c + 3 - width);
                    break;
                }
            }
            fputc('\n', stream); // newline
        }
        fflush(stream);
    }

    static
        int my_trace(CURL* handle, curl_infotype type,
            char* data, size_t size,
            void* userp)
    {
        char config = *reinterpret_cast<char*>(userp);
        const char* text;
        (void)handle; // prevent compiler warning

        switch (type) {
        case CURLINFO_TEXT:
            fprintf(stderr, "== Info: %s", data);
            // FALLTHROUGH
        default: // in case a new one is introduced to shock us 
            return 0;

        case CURLINFO_HEADER_OUT:
            text = "=> Send header";
            break;
        case CURLINFO_DATA_OUT:
            text = "=> Send data";
            break;
        case CURLINFO_SSL_DATA_OUT:
            text = "=> Send SSL data";
            break;
        case CURLINFO_HEADER_IN:
            text = "<= Recv header";
            break;
        case CURLINFO_DATA_IN:
            text = "<= Recv data";
            break;
        case CURLINFO_SSL_DATA_IN:
            text = "<= Recv SSL data";
            break;
        }

        dump(text, stderr, (unsigned char*)data, size, config);
        return 0;
    }
#endif // ANUBIS_NET_ENABLE_DUMP != 0

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

#if ANUBIS_NET_ENABLE_DUMP != 0
        char trace_ascii = 1; // enable ascii tracing
        curl_easy_setopt(m_Session, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(m_Session, CURLOPT_DEBUGFUNCTION, my_trace);
        curl_easy_setopt(m_Session, CURLOPT_DEBUGDATA, &trace_ascii);
#endif // ANUBIS_NET_ENABLE_DUMP != 0
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
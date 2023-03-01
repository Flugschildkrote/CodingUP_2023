#ifndef ANUBIS_CODING_UP_NET_HPP
#define ANUBIS_CODING_UP_NET_HPP

#include <string>
#include <anubis/net/Http.hpp>

namespace anubis
{
namespace coding_up
{

    struct CodingUpData {

        std::string signature;
        std::string data;

        static std::string ToUrlEncodedString(const CodingUpData& d) noexcept {
            return "sig=" + d.signature + "&rep=" + d.data;
        }

        static size_t FromString(std::string_view inStr, CodingUpData& d) noexcept {
            size_t endLinePos = inStr.find_first_of('\n');

            d.signature = std::string(inStr.begin(), inStr.begin() + endLinePos);
            d.data = std::string(inStr.begin()+endLinePos+1, inStr.end());

            return inStr.size();
        }
    };

    class CodingUpNet 
    {

    public:
        CodingUpNet(const std::string& serverUrl, const std::string& get, const std::string& post)
            : m_Session(serverUrl, true), m_GetEndpoint(get), m_PostEndpoint(post), m_Signature("")
        {

        }

        std::string Get(void) 
        {
            CodingUpData data;
            CURLcode status = m_Session.get<CodingUpData>(m_GetEndpoint, data, CodingUpData::FromString);

            if (status != CURLE_OK) {
                throw std::runtime_error("Failed to get");
            }

            m_Signature = data.signature;
            return data.data;
        }

        std::string Post(const std::string& data) 
        {
            if (m_Signature.empty()) {
                throw std::runtime_error("Trying to send data without signature set");
            }

            std::string outRes;
            CodingUpData upData;
            upData.signature = m_Signature;
            upData.data = data;
            CURLcode status = m_Session.post<std::string, CodingUpData>(m_PostEndpoint, upData, CodingUpData::ToUrlEncodedString, outRes, DefaultStringReader, "application/x-www-form-urlencoded");

            if (status != CURLE_OK) {
                throw std::runtime_error("Failed to get");
            }

            return outRes;
        }

    private:

        static size_t DefaultStringReader(std::string_view inData, std::string& outData) noexcept 
        {
            outData = std::string(inData);
            return outData.size();
        }

        net::HTTPSession m_Session;
        std::string m_GetEndpoint;
        std::string m_PostEndpoint;
        std::string m_Signature;
    };

}
}

#endif // !ANUBIS_CODING_UP_NET_HPP
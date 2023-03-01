#ifndef ANUBIS_UTIL_BASE64_HPP
#define ANUBIS_UTIL_BASE64_HPP

#include <vector>
#include <cassert>
#include <string_view>

namespace anubis {
    namespace util {

        std::vector<uint8_t> base64Decode(std::string_view data_base64) {

            assert(((data_base64.length() % 4) == 0) && "Base64 data must be a multiple of 4 bytes in length");

            auto charToValue = [](char v) -> uint8_t {
                if (v >= 'A' && v <= 'Z')
                    return v - 'A';

                if (v >= 'a' && v <= 'z')
                    return v - 'a' + 0b011010;

                if (v >= '0' && v <= '9')
                    return v - '0' + 0b110100;

                if (v == '+')
                    return 0b111110;

                if (v == '/')
                    return 0b111111;

                return 0;
            };

            std::vector<uint8_t> binaryResult;
            //result.reserve(data_base64.length()); // a bit too much but is ok
            auto it = data_base64.begin();
            for (it; it < data_base64.end() - 4; it += 4)
            {
                // decoding is done using 4 characters (=3 bytes)
                uint32_t data = 0; // 6 bits

                for (int32_t i = 0; i < 4; ++i) {
                    data <<= 6;
                    data |= charToValue(*(it + i)); // 6 bits
                }

                binaryResult.push_back(static_cast<uint8_t>(data >> 16));
                binaryResult.push_back(static_cast<uint8_t>(data >> 8));
                binaryResult.push_back(static_cast<uint8_t>(data));
            }

            if (*(it + 2) == '=') { // Decode only one byte (XX==)
                uint8_t data = (*it);
                data <<= 2;
                data |= (*(it + 1) >> 4);
                binaryResult.push_back(data);

            }
            else if (*(it + 3) == '=') { // Decode only two byte (XXX=)
                uint16_t data = (*it);
                data <<= 6;
                data |= (*(it + 1));
                data <<= 6;
                data |= (*(it + 2) >> 2);

                binaryResult.push_back(static_cast<uint8_t>(data >> 8));
                binaryResult.push_back(static_cast<uint8_t>(data));
            }
            else { // Decode full three bytes (XXXX)
                uint32_t data = 0; // 6 bits
                for (int32_t i = 0; i < 4; ++i) {
                    data <<= 6;
                    data |= charToValue(*(it + i)); // 6 bits
                }

                binaryResult.push_back(static_cast<uint8_t>(data >> 16));
                binaryResult.push_back(static_cast<uint8_t>(data >> 8));
                binaryResult.push_back(static_cast<uint8_t>(data));
            }

            return binaryResult;
        }
    }
}
#endif // !
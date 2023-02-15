#include "vdf_utils.h"

#include <sstream>
#include <algorithm>

#include <cstring>

namespace vdf {

namespace utils {

Bytes ConnectBytes(Bytes const& lhs, Bytes const& rhs) {
    Bytes res(lhs.size() + rhs.size());
    memcpy(res.data(), lhs.data(), lhs.size());
    memcpy(res.data() + lhs.size(), rhs.data(), rhs.size());
    return res;
}

uint8_t ValueFromHexChar(char ch) {
    char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    char chlo = std::tolower(ch);
    auto it = std::find(std::begin(hex), std::end(hex), ch);
    if (it == std::end(hex)) {
        throw std::runtime_error("invalid hex character");
    }
    return std::distance(std::begin(hex), it);
}

Bytes BytesFromStr(std::string const& str) {
    if (str.size() % 2 != 0) {
        throw std::runtime_error("invalid hex string, the number of length cannot be divided by 2");
    }
    uint32_t size = str.size() / 2;
    std::vector<uint8_t> res(size);
    for (uint32_t i = 0; i < size; ++i) {
        uint8_t byte = (ValueFromHexChar(str[i * 2]) << 4) + ValueFromHexChar(str[i * 2 + 1]);
        res[i] = byte;
    }
    return res;
}

Bytes GetDefaultForm() {
    Bytes default_form(100, 0);
    default_form[0] = 8;
    return default_form;
}

}  // namespace utils
}  // namespace vdf



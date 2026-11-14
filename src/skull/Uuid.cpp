#include "skull/Uuid.h"

#include <array>
#include <charconv>
#include <cstdio>
#include <random>

namespace custom_skull {

namespace {

uint8_t hexDigit(char c) {
    if (c >= '0' && c <= '9') {
        return static_cast<uint8_t>(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return static_cast<uint8_t>(c - 'a' + 10);
    }
    if (c >= 'A' && c <= 'F') {
        return static_cast<uint8_t>(c - 'A' + 10);
    }
    return 0;
}

} // namespace

mce::UUID parseUuid(std::string const& text) {
    std::array<uint8_t, 16> bytes{};
    size_t                  index = 0;
    bool                    high  = true;
    for (char c : text) {
        if (c == '-') {
            continue;
        }
        if (index >= 16) {
            break;
        }
        uint8_t value = hexDigit(c);
        if (high) {
            bytes[index] = static_cast<uint8_t>(value << 4);
        } else {
            bytes[index] = static_cast<uint8_t>(bytes[index] | value);
            ++index;
        }
        high = !high;
    }
    uint64_t a = 0;
    uint64_t b = 0;
    for (int i = 0; i < 8; ++i) {
        a = (a << 8) | bytes[i];
        b = (b << 8) | bytes[i + 8];
    }
    return mce::UUID(a, b);
}

std::string uuidToString(mce::UUID const& uuid) {
    std::array<uint8_t, 16> bytes{};
    for (int i = 0; i < 8; ++i) {
        bytes[i]     = static_cast<uint8_t>((uuid.a >> (56 - i * 8)) & 0xFF);
        bytes[i + 8] = static_cast<uint8_t>((uuid.b >> (56 - i * 8)) & 0xFF);
    }
    char buffer[37]{};
    std::snprintf(
        buffer,
        sizeof(buffer),
        "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
        bytes[0],
        bytes[1],
        bytes[2],
        bytes[3],
        bytes[4],
        bytes[5],
        bytes[6],
        bytes[7],
        bytes[8],
        bytes[9],
        bytes[10],
        bytes[11],
        bytes[12],
        bytes[13],
        bytes[14],
        bytes[15]
    );
    return std::string(buffer);
}

std::string makeUuid() {
    static std::mt19937_64 generator{std::random_device{}()};
    uint64_t               a = generator();
    uint64_t               b = generator();
    a                        = (a & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;
    b                        = (b & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;
    return uuidToString(mce::UUID(a, b));
}

} // namespace custom_skull

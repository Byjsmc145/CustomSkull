#include "skull/JavaSkin.h"

#include <nlohmann/json.hpp>

#include <cctype>
#include <string>
#include <vector>

#include "skull/Http.h"
#include "skull/Png.h"

namespace custom_skull {

namespace {

std::string urlEncode(std::string const& text) {
    static char const* hex = "0123456789ABCDEF";
    std::string        out;
    out.reserve(text.size() * 3);
    for (unsigned char c : text) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out.push_back(static_cast<char>(c));
        } else {
            out.push_back('%');
            out.push_back(hex[c >> 4]);
            out.push_back(hex[c & 0x0F]);
        }
    }
    return out;
}

int base64Value(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+' || c == '-') return 62;
    if (c == '/' || c == '_') return 63;
    return -1;
}

std::optional<std::string> base64Decode(std::string const& input) {
    std::string output;
    output.reserve(input.size() * 3 / 4);
    int  buffer = 0;
    int  bits   = 0;
    for (char c : input) {
        if (c == '=') {
            break;
        }
        int value = base64Value(c);
        if (value < 0) {
            if (std::isspace(static_cast<unsigned char>(c))) {
                continue;
            }
            return std::nullopt;
        }
        buffer = (buffer << 6) | value;
        bits += 6;
        if (bits >= 8) {
            bits -= 8;
            output.push_back(static_cast<char>((buffer >> bits) & 0xFF));
        }
    }
    return output;
}

std::optional<nlohmann::json> parseJson(std::string const& text) {
    auto value = nlohmann::json::parse(text, nullptr, false);
    if (value.is_discarded() || !value.is_object()) {
        return std::nullopt;
    }
    return value;
}

} // namespace

std::optional<JavaSkinResult> fetchJavaSkin(std::string const& playerName, std::string& error) {
    if (playerName.empty()) {
        error = "empty player name";
        return std::nullopt;
    }

    std::string apiError;
    auto profileBody = httpGet(
        "https://api.mojang.com/users/profiles/minecraft/" + urlEncode(playerName),
        apiError
    );
    if (!profileBody) {
        error = "Mojang profile lookup failed: " + apiError;
        return std::nullopt;
    }
    auto profile = parseJson(*profileBody);
    if (!profile) {
        error = "Mojang profile response is not valid JSON";
        return std::nullopt;
    }
    std::string id = profile->value("id", std::string{});
    std::string name = profile->value("name", playerName);
    if (id.empty()) {
        error = "player not found: " + playerName;
        return std::nullopt;
    }

    auto sessionBody = httpGet(
        "https://sessionserver.mojang.com/session/minecraft/profile/" + id,
        apiError
    );
    if (!sessionBody) {
        error = "Mojang session lookup failed: " + apiError;
        return std::nullopt;
    }
    auto session = parseJson(*sessionBody);
    if (!session) {
        error = "Mojang session response is not valid JSON";
        return std::nullopt;
    }

    std::string encodedTexture;
    if (!session->contains("properties") || !(*session)["properties"].is_array()) {
        error = "Mojang session response has no textures";
        return std::nullopt;
    }
    for (auto const& property : (*session)["properties"]) {
        if (property.is_object() && property.value("name", std::string{}) == "textures") {
            encodedTexture = property.value("value", std::string{});
            break;
        }
    }
    if (encodedTexture.empty()) {
        error = "Mojang session response has no textures property";
        return std::nullopt;
    }

    auto textureJson = base64Decode(encodedTexture);
    if (!textureJson) {
        error = "invalid base64 texture payload";
        return std::nullopt;
    }
    auto textureRoot = parseJson(*textureJson);
    if (!textureRoot) {
        error = "invalid texture JSON";
        return std::nullopt;
    }
    if (!textureRoot->contains("textures") || !(*textureRoot)["textures"].is_object()) {
        error = "texture JSON has no textures object";
        return std::nullopt;
    }
    auto const& textures = (*textureRoot)["textures"];
    if (!textures.contains("SKIN") || !textures["SKIN"].is_object()) {
        error = "player has no skin";
        return std::nullopt;
    }
    auto const& skinNode = textures["SKIN"];
    std::string skinUrl = skinNode.value("url", std::string{});
    if (skinUrl.empty()) {
        error = "player skin URL is empty";
        return std::nullopt;
    }
    bool slim = false;
    if (skinNode.contains("metadata") && skinNode["metadata"].is_object()) {
        slim = skinNode["metadata"].value("model", std::string{}) == "slim";
    }

    auto skinBytes = httpGet(skinUrl, apiError);
    if (!skinBytes) {
        error = "skin download failed: " + apiError;
        return std::nullopt;
    }
    auto image = decodePng(*skinBytes);
    if (!image) {
        error = "downloaded skin is not a valid PNG";
        return std::nullopt;
    }
    image->slim = slim;

    JavaSkinResult result;
    result.image = std::move(*image);
    result.name  = name;
    return result;
}

} // namespace custom_skull

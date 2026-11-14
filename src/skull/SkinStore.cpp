#include "skull/SkinStore.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <system_error>

#include "skull/Png.h"
#include "skull/Uuid.h"

namespace custom_skull {

namespace {

std::string readFile(std::filesystem::path const& path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        return {};
    }
    return std::string(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
}

bool writeFile(std::filesystem::path const& path, std::string const& data) {
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream) {
        return false;
    }
    stream.write(data.data(), static_cast<std::streamsize>(data.size()));
    return static_cast<bool>(stream);
}

} // namespace

SkinStore::SkinStore(std::filesystem::path root)
: mRoot(std::move(root)),
  mIndexPath(mRoot / "skins.json"),
  mImageDir(mRoot / "skinData") {}

bool SkinStore::load(std::string& error) {
    std::error_code code;
    std::filesystem::create_directories(mImageDir, code);
    if (code) {
        error = "cannot create " + mImageDir.string();
        return false;
    }

    mRecords.clear();
    mImageCache.clear();

    if (!std::filesystem::exists(mIndexPath)) {
        return true;
    }
    auto content = readFile(mIndexPath);
    if (content.empty()) {
        return true;
    }
    auto document = nlohmann::json::parse(content, nullptr, false);
    if (document.is_discarded() || !document.is_object()) {
        error = "skins.json is not valid JSON";
        return false;
    }
    auto const& skins = document.contains("skins") ? document["skins"] : nlohmann::json::array();
    if (!skins.is_array()) {
        error = "skins.json has no skins array";
        return false;
    }
    for (auto const& item : skins) {
        if (!item.is_object()) {
            continue;
        }
        SkinRecord record;
        record.uuid   = item.value("uuid", std::string{});
        record.name   = item.value("name", std::string{});
        record.source = item.value("source", std::string{});
        record.file   = item.value("file", std::string{});
        record.slim   = item.value("slim", false);
        record.width  = item.value("width", 0);
        record.height = item.value("height", 0);
        if (record.uuid.empty() || record.name.empty()) {
            continue;
        }
        if (record.file.empty()) {
            record.file = record.uuid + ".png";
        }
        mRecords.push_back(std::move(record));
    }
    return true;
}

bool SkinStore::save(std::string& error) const {
    nlohmann::json document;
    document["version"] = 1;
    auto skins          = nlohmann::json::array();
    for (auto const& record : mRecords) {
        nlohmann::json item;
        item["uuid"]   = record.uuid;
        item["name"]   = record.name;
        item["source"] = record.source;
        item["file"]   = record.file;
        item["slim"]   = record.slim;
        item["width"]  = record.width;
        item["height"] = record.height;
        skins.push_back(std::move(item));
    }
    document["skins"] = std::move(skins);

    std::error_code code;
    std::filesystem::create_directories(mRoot, code);
    if (code) {
        error = "cannot create " + mRoot.string();
        return false;
    }
    if (!writeFile(mIndexPath, document.dump(4))) {
        error = "cannot write " + mIndexPath.string();
        return false;
    }
    return true;
}

SkinRecord const* SkinStore::findByName(std::string_view name, std::string_view source) const {
    for (auto const& record : mRecords) {
        if (record.name == name && record.source == source) {
            return &record;
        }
    }
    return nullptr;
}

SkinRecord const* SkinStore::findByUuid(std::string_view uuid) const {
    for (auto const& record : mRecords) {
        if (record.uuid == uuid) {
            return &record;
        }
    }
    return nullptr;
}

SkinRecord SkinStore::upsert(std::string name, std::string source, SkinImage const& image, std::string& error) {
    if (!image.valid()) {
        error = "invalid skin image";
        return {};
    }

    SkinRecord record;
    if (auto* existing = findByName(name, source)) {
        record = *existing;
    } else {
        record.uuid   = makeUuid();
        record.name   = std::move(name);
        record.source = std::move(source);
        record.file   = record.uuid + ".png";
        mRecords.push_back(record);
    }
    record.slim   = image.slim;
    record.width  = static_cast<int>(image.width);
    record.height = static_cast<int>(image.height);

    auto pngPath = mImageDir / record.file;
    if (!writeFile(pngPath, encodePng(image))) {
        error = "cannot write " + pngPath.string();
        return {};
    }
    mImageCache[record.uuid] = image;

    for (auto& entry : mRecords) {
        if (entry.uuid == record.uuid) {
            entry = record;
            break;
        }
    }
    if (!save(error)) {
        return {};
    }
    return record;
}

std::optional<SkinImage> SkinStore::image(SkinRecord const& record) const {
    if (auto it = mImageCache.find(record.uuid); it != mImageCache.end()) {
        return it->second;
    }
    std::string content = readFile(mImageDir / record.file);
    if (content.empty()) {
        return std::nullopt;
    }
    auto image = decodePng(content);
    if (!image) {
        return std::nullopt;
    }
    image->slim = record.slim;
    mImageCache.emplace(record.uuid, *image);
    return image;
}

} // namespace custom_skull

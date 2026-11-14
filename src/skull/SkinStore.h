#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "skull/Types.h"

namespace custom_skull {

class SkinStore {
public:
    explicit SkinStore(std::filesystem::path root);

    bool load(std::string& error);
    bool save(std::string& error) const;

    SkinRecord const* findByName(std::string_view name, std::string_view source) const;
    SkinRecord const* findByUuid(std::string_view uuid) const;

    SkinRecord upsert(std::string name, std::string source, SkinImage const& image, std::string& error);

    std::vector<SkinRecord> const& all() const { return mRecords; }

    std::optional<SkinImage> image(SkinRecord const& record) const;

    std::filesystem::path const& root() const { return mRoot; }
    std::filesystem::path const& imageDir() const { return mImageDir; }

private:
    std::filesystem::path                    mRoot;
    std::filesystem::path                    mIndexPath;
    std::filesystem::path                    mImageDir;
    std::vector<SkinRecord>                  mRecords;
    mutable std::unordered_map<std::string, SkinImage> mImageCache;
};

} // namespace custom_skull

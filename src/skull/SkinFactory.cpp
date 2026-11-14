#include "skull/SkinFactory.h"

#include "mc/deps/core/container/Blob.h"
#include "mc/deps/core/image/Image.h"
#include "mc/deps/json/Reader.h"
#include "mc/deps/json/Value.h"
#include "mc/deps/shared_types/persona/ArmSizeType.h"
#include "mc/resources/MinEngineVersion.h"
#include "mc/world/actor/player/SerializedSkinImpl.h"
#include "mc/world/actor/player/SkinImage.h"
#include "mc/world/actor/player/TrustedSkinFlag.h"

#include <string>

namespace custom_skull {

namespace {

constexpr std::string_view kSkullGeometry =
    R"({"format_version":"1.10.0","geometry.humanoid.customskull":{"texturewidth":64,"textureheight":64,"visible_bounds_width":2,"visible_bounds_height":1,"visible_bounds_offset":[0,0,0],"bones":[{"name":"head","pivot":[0,24,0],"cubes":[{"origin":[-4,0,-4],"size":[8,8,8],"uv":[0,0]}]},{"name":"hat","parent":"head","pivot":[0,24,0],"cubes":[{"origin":[-4,0,-4],"size":[8,8,8],"uv":[32,0],"inflate":0.5}]}]}})";

} // namespace

std::string_view skullEntityGeometry() { return kSkullGeometry; }

bool buildSerializedSkin(SerializedSkinRef& skin, SkinImage const& image, std::string const& skinId) {
    if (!image.valid()) {
        return false;
    }
    auto defaultSkin = SerializedSkinRef::createTrustedDefaultSerializedSkin();
    skin             = defaultSkin;
    if (!skin.mSkinImpl) {
        return false;
    }

    auto&             impl     = skin.mSkinImpl->mObject;
    std::string const geometry = image.slim ? "geometry.humanoid.customSlim" : "geometry.humanoid.custom";

    impl.mId.get()                  = skinId;
    impl.mFullId.get()              = skinId;
    impl.mResourcePatch.get()       = std::string(R"({"geometry":{"default":")") + geometry + R"("}})";
    impl.mDefaultGeometryName.get() = geometry;
    impl.mCapeId.get()              = "";
    impl.mAnimationData.get()       = "";
    impl.mProfileHash.get()         = "";

    auto& skinImage = impl.mSkinImage.get();
    skinImage.setImageDescription(image.width, image.height, mce::ImageFormat::RGBA8Unorm, mce::ImageUsage::Data);
    skinImage.setRawImage(mce::Blob(image.rgba.data(), image.rgba.size()));

    auto& cape = impl.mCapeImage.get();
    cape.setImageDescription(0, 0, mce::ImageFormat::RGBA8Unorm, mce::ImageUsage::Data);
    cape.setRawImage(mce::Blob{});

    impl.mSkinColor.get()           = mce::Color();
    impl.mArmSizeType               = image.slim ? SharedTypes::persona::ArmSizeType::Slim
                                                 : SharedTypes::persona::ArmSizeType::Wide;
    impl.mIsPremium                 = true;
    impl.mIsPersona                 = false;
    impl.mIsPersonaCapeOnClassicSkin = false;
    impl.mIsPrimaryUser             = false;
    impl.mOverridesPlayerAppearance = false;
    impl.mIsTrustedSkin             = TrustedSkinFlag::True;
    return true;
}

bool buildSkullEntitySkin(SerializedSkinRef& skin, SkinImage const& image, std::string const& skinId) {
    if (!buildSerializedSkin(skin, image, skinId)) {
        return false;
    }

    static constexpr std::string_view geometryJson = kSkullGeometry;

    auto& impl = skin.mSkinImpl->mObject;
    impl.mResourcePatch.get()       = R"({"geometry":{"default":"geometry.humanoid.customskull"}})";
    impl.mDefaultGeometryName.get() = "geometry.humanoid.customskull";

    Json::Reader reader;
    Json::Value  geometry;
    if (!reader.parse(std::string(geometryJson), geometry, false)) {
        return false;
    }
    impl.mGeometryData.get()                 = std::move(geometry);
    impl.mGeometryDataMinEngineVersion.get() = MinEngineVersion::fromString("0.0.0");
    impl.mIsPrimaryUser                      = true;
    impl.mIsTrustedSkin                      = TrustedSkinFlag::True;
    impl.mIsPremium                          = true;
    return true;
}

} // namespace custom_skull

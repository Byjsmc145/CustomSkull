#include "skull/PlayerSkin.h"

#include <cstring>

#include "mc/deps/shared_types/persona/ArmSizeType.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/actor/player/SerializedSkinImpl.h"
#include "mc/world/actor/player/SerializedSkinRef.h"
#include "mc/world/actor/player/SkinImage.h"

namespace custom_skull {

std::optional<SkinImage> imageFromPlayer(Player const& player) {
    if (!player.mSkin || !(*player.mSkin).mSkinImpl) {
        return std::nullopt;
    }
    auto& impl  = (*player.mSkin).mSkinImpl->mObject;
    auto& image = impl.mSkinImage.get();
    if (image.mWidth == 0 || image.mHeight == 0) {
        return std::nullopt;
    }
    auto bytes = image.mImageBytes.getSpan();
    if (bytes.size() != static_cast<size_t>(image.mWidth) * image.mHeight * 4) {
        return std::nullopt;
    }

    SkinImage result;
    result.width  = image.mWidth;
    result.height = image.mHeight;
    result.slim   = impl.mArmSizeType == SharedTypes::persona::ArmSizeType::Slim;
    result.rgba.assign(bytes.begin(), bytes.end());
    return result;
}

} // namespace custom_skull

#include "hologram/HologramBridge.h"

#include <windows.h>

#include <cstdlib>
#include <format>

#include "hologramlib/HologramLib.h"

namespace custom_skull {

HologramBridge& HologramBridge::getInstance() {
    static HologramBridge instance;
    return instance;
}

bool HologramBridge::initialize() {
    if (mNpc) {
        return true;
    }
    mTried = true;

    HMODULE module = GetModuleHandleA("HologramLib.dll");
    if (!module) {
        module = LoadLibraryA("HologramLib.dll");
    }
    if (!module) {
        mError = "HologramLib.dll is not loaded";
        return false;
    }

    using AbiStampFn = uint32_t (*)();
    auto abiStamp    = reinterpret_cast<AbiStampFn>(GetProcAddress(module, "hologramlib_abiStamp"));
    if (!abiStamp) {
        mError = "HologramLib ABI stamp is missing";
        return false;
    }
    uint32_t runtimeStamp = abiStamp();
    if (runtimeStamp == 0 || runtimeStamp > HOLOGLIB_ABI_STAMP) {
        mError = std::format(
            "HologramLib ABI stamp mismatch: runtime {}, expected up to {}",
            runtimeStamp,
            HOLOGLIB_ABI_STAMP
        );
        return false;
    }

    using GetInstanceFn = hologramlib::IHologramLib& (*)();
    auto getInstance   = reinterpret_cast<GetInstanceFn>(
        GetProcAddress(module, "?getInstance@IHologramLib@hologramlib@@SAAEAV12@XZ")
    );
    if (!getInstance) {
        mError = "HologramLib getInstance export is missing";
        return false;
    }

    mModule = module;
    mLib    = &getInstance();
    if (!mLib) {
        mError = "HologramLib getInstance returned null";
        return false;
    }

    auto version = mLib->version();
    if (version < 0x012000u) {
        mError = std::format("HologramLib API version is too old: 0x{:08x}", version);
        mLib   = nullptr;
        return false;
    }

    mNpc = &mLib->playerNpcs();
    if (!mNpc) {
        mError = "HologramLib playerNpcs() returned null";
        mLib   = nullptr;
        return false;
    }
    return true;
}

bool HologramBridge::registerSkin(
    std::string const& pngPath,
    std::string const& skinId,
    std::string const& geometryData
) {
    if (!mNpc) {
        return false;
    }
    hologramlib::PlayerNpcSkin skin;
    skin.pngPath      = pngPath;
    skin.skinId       = skinId;
    skin.geometry     = "geometry.humanoid.customskull";
    skin.armSize      = "wide";
    skin.geometryData = geometryData;
    return mNpc->registerSkin(skin);
}

bool HologramBridge::hasSkin(std::string const& skinId) {
    return mNpc && mNpc->hasSkin(skinId);
}

int64_t HologramBridge::create(
    std::string const& skinId,
    std::string const& name,
    float              x,
    float              y,
    float              z,
    int                dimension,
    float              yaw,
    float              scale
) {
    if (!mNpc) {
        return -1;
    }
    hologramlib::PlayerNpcConfig config;
    config.name         = name;
    config.skinId       = skinId;
    config.x            = x;
    config.y            = y;
    config.z            = z;
    config.dimension    = dimension;
    config.yaw          = yaw;
    config.viewDistance = 96.0;
    config.scale        = scale;
    config.enabled      = true;
    return mNpc->create(config);
}

bool HologramBridge::destroy(int64_t id) {
    return mNpc && id >= 0 && mNpc->destroy(id);
}

bool HologramBridge::setNametag(int64_t id, std::string const& text) {
    return mNpc && id >= 0 && mNpc->setNametag(id, text);
}

bool HologramBridge::setSkin(int64_t id, std::string const& skinId) {
    return mNpc && id >= 0 && mNpc->setSkin(id, skinId);
}

bool HologramBridge::setRotation(int64_t id, float yaw) {
    return mNpc && id >= 0 && mNpc->setRotation(id, yaw);
}

bool HologramBridge::setRotationLight(int64_t id, float yaw) {
    return mNpc && id >= 0 && mNpc->setRotationLight(id, yaw);
}

bool HologramBridge::setPosition(int64_t id, float x, float y, float z, int dimension) {
    return mNpc && id >= 0 && mNpc->setPosition(id, x, y, z, dimension);
}

bool HologramBridge::setPositionLight(int64_t id, float x, float y, float z, int dimension) {
    return mNpc && id >= 0 && mNpc->setPositionLight(id, x, y, z, dimension);
}

int64_t HologramBridge::createAnchor(float x, float y, float z, int dimension) {
    if (!mLib) {
        return -1;
    }
    hologramlib::CustomEntityConfig config;
    config.identifier        = "minecraft:armor_stand";
    config.x                 = x;
    config.y                 = y;
    config.z                 = z;
    config.dimension         = dimension;
    config.invisible         = true;
    config.viewDistance      = 96.0;
    config.enabled           = true;
    return mLib->customEntities().create(config);
}

bool HologramBridge::rideEntity(int64_t npcId, int64_t anchorId) {
    return mNpc && npcId >= 0 && anchorId >= 0 && mNpc->setRideEntity(npcId, anchorId);
}

bool HologramBridge::destroyAnchor(int64_t anchorId) {
    return mLib && anchorId >= 0 && mLib->customEntities().destroy(anchorId);
}

std::string HologramBridge::debugInfo(int64_t id) {
    return mNpc && id >= 0 ? mNpc->getDebugInfo(id) : std::string{};
}

bool HologramBridge::shrinkHitbox(int64_t id) {
    if (!mLib || !mNpc || id < 0) {
        return false;
    }
    auto info = mNpc->getDebugInfo(id);
    auto pos  = info.find("uniqueId=0x");
    if (pos == std::string::npos) {
        return false;
    }
    auto value = std::strtoull(info.c_str() + pos + 11, nullptr, 16);
    if (value == 0) {
        return false;
    }
    hologramlib::EntityView view;
    view.hasMetaScale     = true;
    view.metaScale        = 0.001f;
    view.hasNametag       = true;
    view.nametag          = "\xC2\xA7r";
    view.nametagAlwaysShow = true;
    return mLib->viewOverrides().overrideEntity("", static_cast<int64_t>(value), view);
}

bool HologramBridge::overrideSkin(int64_t uniqueId, std::string const& skinId, float yOffset) {
    if (!mLib || uniqueId <= 0) {
        return false;
    }
    hologramlib::EntityView view;
    view.identifier         = "minecraft:armor_stand";
    view.skinId             = skinId;
    view.scale              = 1.0f;
    view.yOffset            = yOffset;
    view.hasNametag         = true;
    view.nametag            = "\xC2\xA7r";
    view.nametagAlwaysShow  = true;
    view.hasMetaScale       = true;
    view.metaScale          = 0.001f;
    return mLib->viewOverrides().overrideEntity("", uniqueId, view);
}

bool HologramBridge::clearOverride(int64_t uniqueId) {
    return mLib && uniqueId > 0 && mLib->viewOverrides().clearEntity("", uniqueId);
}

} // namespace custom_skull

#pragma once

#include <cstdint>
#include <string>

namespace hologramlib {
class IHologramLib;
class IPlayerNpc;
struct PlayerNpcConfig;
struct PlayerNpcSkin;
} // namespace hologramlib

namespace custom_skull {

class HologramBridge {
public:
    static HologramBridge& getInstance();

    bool initialize();
    [[nodiscard]] bool available() const { return mNpc != nullptr; }
    [[nodiscard]] std::string const& lastError() const { return mError; }

    bool registerSkin(std::string const& pngPath, std::string const& skinId, std::string const& geometryData);
    bool hasSkin(std::string const& skinId);

    int64_t create(
        std::string const& skinId,
        std::string const& name,
        float              x,
        float              y,
        float              z,
        int                dimension,
        float              yaw,
        float              scale
    );

    bool destroy(int64_t id);
    bool setNametag(int64_t id, std::string const& text);
    bool setSkin(int64_t id, std::string const& skinId);
    bool setRotation(int64_t id, float yaw);
    bool setRotationLight(int64_t id, float yaw);
    bool setPosition(int64_t id, float x, float y, float z, int dimension);
    bool setPositionLight(int64_t id, float x, float y, float z, int dimension);

    int64_t createAnchor(float x, float y, float z, int dimension);
    bool    rideEntity(int64_t npcId, int64_t anchorId);
    bool    destroyAnchor(int64_t anchorId);

    std::string debugInfo(int64_t id);
    bool        shrinkHitbox(int64_t id);

    bool overrideSkin(int64_t uniqueId, std::string const& skinId, float yOffset);
    bool clearOverride(int64_t uniqueId);

private:
    HologramBridge() = default;

    bool                          mTried = false;
    void*                         mModule = nullptr;
    hologramlib::IHologramLib*    mLib    = nullptr;
    hologramlib::IPlayerNpc*      mNpc    = nullptr;
    std::string                   mError;
};

} // namespace custom_skull

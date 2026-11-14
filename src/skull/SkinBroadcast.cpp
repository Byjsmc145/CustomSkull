#include "skull/SkinBroadcast.h"

#include <functional>
#include <string>

#include "mc/deps/core/platform/BuildPlatform.h"
#include "mc/network/packet/PlayerListPacket.h"
#include "mc/network/packet/PlayerListPacketPayload.h"
#include "mc/network/packet/PlayerListPacketType.h"
#include "mc/network/packet/PlayerSkinPacket.h"
#include "mc/network/packet/cerealize/core/SerializationMode.h"
#include "mc/world/actor/player/Player.h"
#include "skull/SkinFactory.h"
#include "skull/Uuid.h"

namespace custom_skull {

namespace {

int64 entityIdFor(std::string const& uuid) {
    auto hash = static_cast<uint64>(std::hash<std::string>{}(uuid));
    return static_cast<int64>(hash & 0x7FFFFFFFFFFFFFFFULL);
}

void appendEntry(PlayerListPacket& packet, SkinPayload const& payload, SerializedSkinRef const& skin) {
    PlayerListPacketPayload::AddEntry entry;
    entry.mUUID             = parseUuid(payload.record.uuid);
    entry.mId.get().rawID   = entityIdFor(payload.record.uuid);
    entry.mName             = "Skull-" + payload.record.uuid.substr(0, 8);
    entry.mXUID             = "";
#if !defined(CS_LL_MINOR) || CS_LL_MINOR >= 60
    entry.mPlayFabId        = "";
#endif
    entry.mPlatformOnlineId = "";
    entry.mBuildPlatform    = BuildPlatform::Dedicated;
    entry.mSkin             = skin;
    entry.mIsTeacher        = false;
    entry.mIsHost           = false;
    entry.mIsSubClient      = false;
    entry.mColor            = mce::Color(170, 170, 170, 255);
    packet.mEntries.get().emplace_back(std::move(entry));
}

void sendPlayerSkin(SkinPayload const& payload, SerializedSkinRef const& skin, Player const* target) {
    PlayerSkinPacket packet;
    packet.mSerializationMode     = SerializationMode::CerealOnly;
    packet.mUUID                 = parseUuid(payload.record.uuid);
    packet.mSkin                     = skin;
    packet.mLocalizedNewSkinName     = "";
    packet.mLocalizedOldSkinName     = "";
    if (target) {
        packet.sendTo(*target);
    } else {
        packet.sendToClients();
    }
}

void sendChunk(std::vector<SkinPayload> const& skins, size_t begin, size_t end, Player const* target, bool keepListed) {
    PlayerListPacket add;
    add.setSerializationMode(SerializationMode::CerealOnly);

    std::vector<size_t> indices;
    indices.reserve(end - begin);

    for (size_t i = begin; i < end; ++i) {
        SerializedSkinRef skin;
        if (!buildSerializedSkin(skin, skins[i].image, skins[i].record.uuid + "_skull")) {
            continue;
        }
        appendEntry(add, skins[i], skin);
        indices.push_back(i);
    }

    if (add.mEntries.get().empty()) {
        return;
    }
    if (target) {
        add.sendTo(*target);
    } else {
        add.sendToClients();
    }

    for (size_t index : indices) {
        SerializedSkinRef skin;
        if (buildSerializedSkin(skin, skins[index].image, skins[index].record.uuid + "_skull")) {
            sendPlayerSkin(skins[index], skin, target);
        }
    }

    if (keepListed) {
        return;
    }

    PlayerListPacket remove;
    remove.setSerializationMode(SerializationMode::CerealOnly);
    for (size_t i = begin; i < end; ++i) {
        PlayerListPacketPayload::RemoveEntry entry;
        entry.mAction = PlayerListPacketType::Remove;
        entry.mUUID   = parseUuid(skins[i].record.uuid);
        remove.mEntries.get().emplace_back(std::move(entry));
    }
    if (target) {
        remove.sendTo(*target);
    } else {
        remove.sendToClients();
    }
}

} // namespace

void sendSkinPayloads(std::vector<SkinPayload> const& skins, Player const* target, int maxPerPacket, bool keepListed) {
    if (skins.empty()) {
        return;
    }
    size_t chunkSize = maxPerPacket <= 0 ? 8 : static_cast<size_t>(maxPerPacket);
    for (size_t begin = 0; begin < skins.size(); begin += chunkSize) {
        size_t end = begin + chunkSize;
        if (end > skins.size()) {
            end = skins.size();
        }
        sendChunk(skins, begin, end, target, keepListed);
    }
}

} // namespace custom_skull

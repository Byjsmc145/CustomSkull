#pragma once

#include <string>
#include <vector>

#include "skull/Types.h"

class Player;

namespace custom_skull {

struct SkinPayload {
    SkinRecord record;
    SkinImage  image;
};

void sendSkinPayloads(std::vector<SkinPayload> const& skins, Player const* target, int maxPerPacket, bool keepListed);

} // namespace custom_skull


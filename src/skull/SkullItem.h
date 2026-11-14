#pragma once

#include <string>

#include "skull/Types.h"

class Player;
class ItemStack;

namespace custom_skull {

std::string formatSkullName(std::string const& format, std::string const& playerName);
ItemStack   createSkullItem(SkinRecord const& record, int count, std::string const& displayName);
bool        giveSkull(Player& player, SkinRecord const& record, int count, std::string const& displayName);

} // namespace custom_skull

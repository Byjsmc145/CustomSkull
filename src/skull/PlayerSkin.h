#pragma once

#include <optional>

#include "skull/Types.h"

class Player;

namespace custom_skull {

std::optional<SkinImage> imageFromPlayer(Player const& player);

} // namespace custom_skull

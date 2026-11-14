#pragma once

#include <string>

#include "mc/platform/UUID.h"

namespace custom_skull {

std::string makeUuid();
mce::UUID   parseUuid(std::string const& text);
std::string uuidToString(mce::UUID const& uuid);

} // namespace custom_skull

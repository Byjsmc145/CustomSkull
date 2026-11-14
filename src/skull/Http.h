#pragma once

#include <optional>
#include <string>

namespace custom_skull {

std::optional<std::string> httpGet(std::string const& url, std::string& error, int timeoutMs = 15000);

} // namespace custom_skull

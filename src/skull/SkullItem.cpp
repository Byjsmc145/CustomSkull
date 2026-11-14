#include "skull/SkullItem.h"

#include "mc/deps/nbt/CompoundTag.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/item/ItemStack.h"

namespace custom_skull {

std::string formatSkullName(std::string const& format, std::string const& playerName) {
    std::string result = format;
    auto        pos    = result.find("{}");
    if (pos != std::string::npos) {
        result.replace(pos, 2, playerName);
    }
    return result;
}

ItemStack createSkullItem(SkinRecord const& record, int count, std::string const& displayName) {
    ItemStack stack;
    stack.reinit("minecraft:skull", count, 3);

    CompoundTag tag;
    tag.putString("SkullOwner", record.uuid);
    CompoundTag display;
    display.putString("Name", displayName);
    tag.putCompound("display", std::move(display));
    tag.putInt("RepairCost", 0);
    stack.setUserData(std::make_unique<CompoundTag>(std::move(tag)));
    return stack;
}

bool giveSkull(Player& player, SkinRecord const& record, int count, std::string const& displayName) {
    auto stack = createSkullItem(record, count, displayName);
    return player.addAndRefresh(stack);
}

} // namespace custom_skull

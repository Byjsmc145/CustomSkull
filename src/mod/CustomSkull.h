#pragma once

#include <memory>
#include <string>

#include "ll/api/event/Listener.h"
#include "ll/api/mod/NativeMod.h"

#include "mod/Config.h"
#include "skull/SkinStore.h"
#include "skull/Types.h"

class Player;

namespace custom_skull {

class SkullEntityManager;

class CustomSkull {
public:
    static CustomSkull& getInstance();

    CustomSkull();

    [[nodiscard]] ll::mod::NativeMod& getSelf() const { return mSelf; }
    [[nodiscard]] ll::io::Logger&     logger() { return mSelf.getLogger(); }

    bool load();
    bool enable();
    bool disable();
    bool unload();

    [[nodiscard]] Config&     config() { return mConfig; }
    [[nodiscard]] SkinStore&  store() { return *mStore; }
    [[nodiscard]] SkullEntityManager& entities() { return *mEntities; }

    void broadcastSkin(SkinRecord const& record, SkinImage const& image);
    void broadcastAllSkins();
    void sendAllSkins(Player& player);

private:
    void onPlayerJoin(Player& player);

    ll::mod::NativeMod&                      mSelf;
    Config                                   mConfig;
    std::unique_ptr<SkinStore>               mStore;
    std::unique_ptr<SkullEntityManager>      mEntities;
    std::shared_ptr<ll::event::ListenerBase> mJoinListener;
};

void registerSkullCommand(CustomSkull& mod);

} // namespace custom_skull

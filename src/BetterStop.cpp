#include "BetterStop.h"

#include "ll/api/command/CommandHandle.h"
#include "ll/api/command/CommandRegistrar.h"
#include "ll/api/event/EventBus.h"
#include "ll/api/event/server/ServerStoppingEvent.h"
#include "ll/api/mod/RegisterHelper.h"
#include "ll/api/service/Bedrock.h"
#include "mc/server/DedicatedServer.h"
#include "mc/server/commands/CommandPermissionLevel.h"
#include "mc/server/commands/CommandRawText.h"
#include "mc/server/commands/StopCommand.h"
#include "mc/world/actor/player/Player.h"
#include "mc/world/level/Level.h"


#include "windows.h"


namespace better_stop {
BetterStop& BetterStop::getInstance() {
    static BetterStop instance;
    return instance;
}
std::string reason;
void        ListenEvents() {
    ll::event::EventBus::getInstance().emplaceListener<ll::event::ServerStoppingEvent>(
        [](ll::event::ServerStoppingEvent& event) {
            ll::service::getLevel()->forEachPlayer([&](Player& pl) -> bool {
                std::string res = "服务器已关闭";
                if (!reason.empty()) res += "\n原因：" + reason;
                pl.disconnect(res);
                return true;
            });
        }
    );
}

struct cmdParam {
    CommandRawText reason;
};
void registerCommand() {
    auto& cmd = ll::command::CommandRegistrar::getInstance()
                    .getOrCreateCommand("bstop", "更好地关闭服务器", CommandPermissionLevel::Host);
    cmd.alias("bs");

    cmd.overload<cmdParam>().optional("reason").execute(
        [](CommandOrigin const& origin, CommandOutput& output, cmdParam const& param) {
            if (!param.reason.mText.empty()) reason = param.reason.mText;
            StopCommand::mServer()->requestServerShutdown("");
        }
    );
}
bool BetterStop::load() {
    HWND  hwnd  = GetConsoleWindow();
    HMENU hmenu = GetSystemMenu(hwnd, false);
    RemoveMenu(hmenu, SC_CLOSE, MF_BYCOMMAND);
    return true;
}
bool BetterStop::enable() {
    ListenEvents();
    registerCommand();
    return true;
}

bool BetterStop::disable() { return true; }

} // namespace better_stop

LL_REGISTER_MOD(better_stop::BetterStop, better_stop::BetterStop::getInstance());

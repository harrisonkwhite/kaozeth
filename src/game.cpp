#include "game.h"

#include "game_consts.h"
#include "assets.h"

static void GameSetState(t_game *const game, const t_game_state state, zgl::t_gfx *const gfx, const zgl::t_platform *const platform, zcl::t_arena *const arena) {
    ZCL_ASSERT(state != ek_game_state_none);

    switch (game->state) {
    case ek_game_state_none:
        break;

    case ek_game_state_title_screen:
        TitleScreenDestroy(&game->state_data.title_screen);
        break;

    case ek_game_state_world:
        WorldDestroy(&game->state_data.world, gfx);
        break;

    default:
        ZCL_UNREACHABLE();
    }

    game->state = state;

    switch (game->state) {
    case ek_game_state_title_screen:
        game->state_data.title_screen = TitleScreenCreate();
        break;

    case ek_game_state_world:
        game->state_data.world = WorldCreate(gfx, platform, arena);
        break;

    default:
        ZCL_UNREACHABLE();
    }
}

void GameInit(const zgl::t_game_init_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    zgl::WindowSetTitle(zf_context.platform, g_game_title, zf_context.temp_arena);

    game->assets = AssetsLoadAll(zf_context.gfx, zf_context.perm_arena, zf_context.temp_arena);

    GameSetState(game, ek_game_state_title_screen, zf_context.gfx, zf_context.platform, zf_context.perm_arena);
}

void GameDeinit(const zgl::t_game_deinit_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    switch (game->state) {
    case ek_game_state_title_screen:
        TitleScreenDestroy(&game->state_data.title_screen);
        break;

    case ek_game_state_world:
        WorldDestroy(&game->state_data.world, zf_context.gfx);
        break;

    default:
        ZCL_UNREACHABLE();
    }

    AssetsUnloadAll(&game->assets, zf_context.gfx);
}

void GameTick(const zgl::t_game_tick_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    switch (game->state) {
    case ek_game_state_title_screen: {
        const t_title_screen_tick_request request = TitleScreenTick(&game->state_data.title_screen, zf_context);

        switch (request) {
        case ek_title_screen_tick_request_none:
            break;

        case ek_title_screen_tick_request_go_to_world:
            GameSetState(game, ek_game_state_world, zf_context.gfx, zf_context.platform, zf_context.perm_arena);
            break;

        default:
            ZCL_UNREACHABLE();
        }

        break;
    }

    case ek_game_state_world: {
        WorldTick(&game->state_data.world, zf_context);
        break;
    }

    default: {
        ZCL_UNREACHABLE();
    }
    }
}

void GameRender(const zgl::t_game_render_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    zgl::FramePassBegin(zf_context.frame_context, zgl::FrameGetSize(zf_context.frame_context));
    zgl::FramePassEnd(zf_context.frame_context);

    switch (game->state) {
    case ek_game_state_title_screen:
        TitleScreenRender(&game->state_data.title_screen, zf_context.frame_context, &game->assets, zf_context.temp_arena);
        break;

    case ek_game_state_world:
        WorldRender(&game->state_data.world, zf_context.frame_context, &game->assets, zf_context.temp_arena);
        break;

    default:
        ZCL_UNREACHABLE();
    }
}

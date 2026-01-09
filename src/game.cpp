#include "game.h"

#include "game_consts.h"
#include "assets.h"

static void game_set_state(t_game *const game, const t_game_state state, zf::mem::t_arena *const arena) {
    ZF_ASSERT(state != ek_game_state_none);

    switch (game->state) {
    case ek_game_state_none:
        break;

    case ek_game_state_title_screen:
        title_screen_deinit(&game->state_data.title_screen);
        break;

    case ek_game_state_world:
        world::destroy(&game->state_data.world);
        break;

    default:
        ZF_UNREACHABLE();
    }

    game->state = state;

    switch (game->state) {
    case ek_game_state_title_screen:
        game->state_data.title_screen = title_screen_init();
        break;

    case ek_game_state_world:
        game->state_data.world = world::create(arena);
        break;

    default:
        ZF_UNREACHABLE();
    }
}

void game_init(const zgl::game::t_init_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    zgl::platform::window_set_title(g_game_title, zf_context.temp_arena);

    assets::load_all(zf_context.perm_arena, zf_context.temp_arena);

    game_set_state(game, ek_game_state_title_screen, zf_context.perm_arena);
}

void game_deinit(void *const user_mem) {
    const auto game = static_cast<t_game *>(user_mem);

    switch (game->state) {
    case ek_game_state_title_screen:
        title_screen_deinit(&game->state_data.title_screen);
        break;

    case ek_game_state_world:
        world::destroy(&game->state_data.world);
        break;

    default:
        ZF_UNREACHABLE();
    }

    assets::unload_all();
}

void game_tick(const zgl::game::t_tick_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    switch (game->state) {
    case ek_game_state_title_screen: {
        const t_title_screen_tick_request request = title_screen_tick(&game->state_data.title_screen, zf_context);

        switch (request) {
        case ek_title_screen_tick_request_none:
            break;

        case ek_title_screen_tick_request_go_to_world:
            game_set_state(game, ek_game_state_world, zf_context.perm_arena);
            break;

        default:
            ZF_UNREACHABLE();
        }

        break;
    }

    case ek_game_state_world: {
        world::tick(&game->state_data.world, zf_context);
        break;
    }

    default: {
        ZF_UNREACHABLE();
    }
    }
}

void game_render(const zgl::game::t_render_func_context &zf_context) {
    const auto game = static_cast<t_game *>(zf_context.user_mem);

    switch (game->state) {
    case ek_game_state_title_screen:
        title_screen_render(&game->state_data.title_screen, zf_context.frame_context, zf_context.temp_arena);
        break;

    case ek_game_state_world:
        world::render(&game->state_data.world, zf_context.frame_context, zf_context.temp_arena);
        break;

    default:
        ZF_UNREACHABLE();
    }
}

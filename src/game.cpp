#include "game.h"

using namespace zf; // @todo

static const zf::s_v2_i g_player_size = {32, 32};

static zf::s_v2 g_pos;

void init_game(const zf::s_game_init_context &zf_context) {
    zf::platform::set_window_title(ZF_STR_LITERAL("Sadness"), zf_context.temp_arena);
    zf::platform::set_cursor_visibility(false);
}

void deinit_game() {
}

void run_game_tick(const zf::s_game_tick_context &zf_context) {
    if (zf::input::get_key_is_down(zf_context.input_state, zf::input::ec_key_code_right)) {
        g_pos.x++;
    }
}

void render_game(const zf::s_game_render_context &zf_context) {
}

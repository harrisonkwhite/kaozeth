#include "title_screen.h"

ec_title_screen_tick_result TitleScreenTick() {
    if (zf::c_window::IsKeyPressed(zf::ek_key_code_enter)) {
        return ec_title_screen_tick_result::go_to_world;
    }

    return ec_title_screen_tick_result::success;
}

void RenderTitleScreen(const s_title_screen& ts, const zf::c_renderer& renderer) {

}

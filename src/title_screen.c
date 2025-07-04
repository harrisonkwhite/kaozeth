#include "game.h"
#include "zfw_utils.h"

#define PAGE_MEM_ARENA_SIZE (1 << 10)

#define BUTTON_FONT ek_font_eb_garamond_28
#define BUTTON_GAP 64.0f

static inline s_button* GetButton(const s_buttons* const buttons, const int index) {
    assert(buttons);
    assert(index >= 0 && index < buttons->cnt);

    return &buttons->buf[index];
}

static s_buttons PushPageButtons(s_mem_arena* const mem_arena, const e_title_screen_page page) {
    AssertMemArenaValidity(mem_arena);

    s_buttons btns;

    switch (page) {
        case ek_title_screen_page_home: btns.cnt = 3; break;
        case ek_title_screen_page_worlds: btns.cnt = 1; break;
        case ek_title_screen_page_options: btns.cnt = 1; break;
    }

    btns.buf = MEM_ARENA_PUSH_TYPE_MANY(mem_arena, s_button, btns.cnt);

    if (!btns.buf) {
        return (s_buttons){0};
    }

    switch (page) {
        case ek_title_screen_page_home:
            GetButton(&btns, 0)->str = "Play";
            GetButton(&btns, 1)->str = "Settings";
            GetButton(&btns, 2)->str = "Exit";
            break;

        case ek_title_screen_page_worlds:
            GetButton(&btns, 0)->str = "Back";
            break;

        case ek_title_screen_page_options:
            GetButton(&btns, 0)->str = "Back";
            break;
    }

    return btns;
}

bool InitTitleScreen(s_title_screen* const ts) {
    assert(IS_ZERO(*ts));

    if (!InitMemArena(&ts->page_mem_arena, PAGE_MEM_ARENA_SIZE)) {
        return false;
    }

    ts->btns = PushPageButtons(&ts->page_mem_arena, ek_title_screen_page_home);

    if (IS_ZERO(ts->btns)) {
        return false;
    }

    return true;
}

void CleanTitleScreen(s_title_screen* const ts) {
    CleanMemArena(&ts->page_mem_arena);
}

bool TitleScreenTick(s_title_screen* const ts) {
    return true;
}

bool RenderTitleScreen(const s_rendering_context* const rendering_context, const s_title_screen* const ts, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_vec_2d_i ui_size = UISize(rendering_context->display_size);

    const float btn_ver_span = (ts->btns.cnt - 1) * BUTTON_GAP;
    const float btns_top = (ui_size.y - btn_ver_span) / 2.0f;

    for (int i = 0; i < ts->btns.cnt; i++) {
        const s_button* const btn = GetButton(&ts->btns, i);
        const s_vec_2d btn_pos = {
            ui_size.x / 2.0f,
            btns_top + (BUTTON_GAP * i),
        };

        if (!RenderStr(rendering_context, btn->str, BUTTON_FONT, fonts, btn_pos, ek_str_hor_align_center, ek_str_ver_align_center, WHITE, temp_mem_arena)) {
            return false;
        }
    }

    return true;
}

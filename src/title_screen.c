#include "game.h"
#include "zfw_utils.h"

#define PAGE_MEM_ARENA_SIZE (1 << 10)

#define BUTTON_FONT ek_font_eb_garamond_28
#define BUTTON_GAP 64.0f

static int g_page_menu_button_cnts[] = {
    [ek_title_screen_page_home] = 3,
    [ek_title_screen_page_worlds] = 1,
    [ek_title_screen_page_options] = 1
};

static_assert(STATIC_ARRAY_LEN(g_page_menu_button_cnts) == eks_title_screen_page_cnt, "Invalid array length!");

static const char* PageButtonStr(const int index, const e_title_screen_page page) {
    const int btn_cnt = g_page_menu_button_cnts[page];

    assert(index >= 0 && index < btn_cnt);

    switch (page) {
        case ek_title_screen_page_home:
            switch (index) {
                case 0: return "Play";
                case 1: return "Settings";
                case 2: return "Exit";
            }

            break;

        case ek_title_screen_page_worlds:
            switch (index) {
                case 0: return "Back";
            }

            break;

        case ek_title_screen_page_options:
            switch (index) {
                case 0: return "Back";
            }

            break;
    }

    assert(false && "Unset button string!");
    return NULL;
}

#if 0
static inline s_button* GetButton(const s_buttons* const buttons, const int index) {
    assert(buttons);
    assert(index >= 0 && index < buttons->cnt);

    return &buttons->buf[index];
}
#endif

static s_buttons PushPageButtons(s_mem_arena* const mem_arena, const e_title_screen_page page) {
    AssertMemArenaValidity(mem_arena);

    s_buttons btns = {
        .cnt = g_page_menu_button_cnts[page]
    };

    btns.buf = MEM_ARENA_PUSH_TYPE_MANY(mem_arena, s_button, btns.cnt);

    if (!btns.buf) {
        return (s_buttons){0};
    }

    for (int i = 0; i < btns.cnt; i++) {
        btns.buf[i].str = PageButtonStr(i, page);
    }

    return btns;
}

static s_vec_2d PageButtonPos(const int btn_index, const e_title_screen_page page, const s_vec_2d_i ui_size) {
    assert(ui_size.x > 0 && ui_size.y > 0);

    const int btn_cnt = g_page_menu_button_cnts[page];

    assert(btn_index >= 0 && btn_index < btn_cnt);

    const float btn_ver_span = (btn_cnt - 1) * BUTTON_GAP;
    const float btns_top = (ui_size.y - btn_ver_span) / 2.0f;
    return (s_vec_2d){
        ui_size.x / 2.0f,
        btns_top + (BUTTON_GAP * btn_index)
    };
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

    ts->btn_hovered_index = -1;

    return true;
}

void CleanTitleScreen(s_title_screen* const ts) {
    CleanMemArena(&ts->page_mem_arena);
}

bool TitleScreenTick(s_title_screen* const ts, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_vec_2d_i ui_size = UISize(display_size);
    const s_vec_2d cursor_ui_pos = DisplayToUIPos(input_state->mouse_pos);

    ts->btn_hovered_index = -1;

    for (int i = 0; i < ts->btns.cnt; i++) {
        const s_button* const btn = &ts->btns.buf[i];
        const s_vec_2d btn_pos = PageButtonPos(i, ts->page, ui_size);
        const char* const btn_str = PageButtonStr(i, ts->page);

        s_rect btn_str_collider = {0};

        if (!LoadStrCollider(&btn_str_collider, btn_str, BUTTON_FONT, fonts, btn_pos, ek_str_hor_align_center, ek_str_ver_align_center, temp_mem_arena)) {
            return false;
        }

        if (IsPointInRect(cursor_ui_pos, btn_str_collider)) {
            ts->btn_hovered_index = i;
        }
    }

    return true;
}

bool RenderTitleScreen(const s_rendering_context* const rendering_context, const s_title_screen* const ts, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_vec_2d_i ui_size = UISize(rendering_context->display_size);

    for (int i = 0; i < ts->btns.cnt; i++) {
        const s_button* const btn = &ts->btns.buf[i];
        const s_vec_2d btn_pos = PageButtonPos(i, ts->page, ui_size);
        const s_color btn_color = ts->btn_hovered_index == i ? YELLOW : WHITE;

        if (!RenderStr(rendering_context, btn->str, BUTTON_FONT, fonts, btn_pos, ek_str_hor_align_center, ek_str_ver_align_center, btn_color, temp_mem_arena)) {
            return false;
        }
    }

    return true;
}

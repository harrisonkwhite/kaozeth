#include "game.h"

#define BUTTON_FONT ek_font_eb_garamond_28
#define BUTTON_GAP 64.0f

typedef enum {
    ek_page_button_type_redirect,
    ek_page_button_type_exit,
    ek_page_button_type_new_world,
    ek_page_button_type_load_world
} e_page_button_type;

typedef struct {
    const char* str;
    e_page_button_type type;
    e_title_screen_page redir_page;
} s_page_button;

#if 0
typedef struct {
    int page_index;
} s_page_button_redirect_type_data;

typedef union {
    s_page_button_redirect_type_data redir;
} u_page_button_type_data;
#endif

static int g_page_button_cnts[] = {
    [ek_title_screen_page_home] = 3,
    [ek_title_screen_page_worlds] = 2,
    [ek_title_screen_page_settings] = 1
};

static_assert(STATIC_ARRAY_LEN(g_page_button_cnts) == eks_title_screen_page_cnt, "Invalid array length!");

static s_page_button* PushPageButtons(s_mem_arena* const mem_arena, const e_title_screen_page page) {
    AssertMemArenaValidity(mem_arena);

    const int btn_cnt = g_page_button_cnts[page];

    s_page_button* const btns = MEM_ARENA_PUSH_TYPE_MANY(mem_arena, s_page_button, btn_cnt);

    if (btns) {
        switch (page) {
            case ek_title_screen_page_home:
                btns[0] = (s_page_button){
                    .str = "Play",
                    .type = ek_page_button_type_redirect,
                    .redir_page = ek_title_screen_page_worlds
                };

                btns[1] = (s_page_button){
                    .str = "Settings",
                    .type = ek_page_button_type_redirect,
                    .redir_page = ek_title_screen_page_settings
                };

                btns[2] = (s_page_button){
                    .str = "Exit",
                    .type = ek_page_button_type_exit
                };

                break;

            case ek_title_screen_page_worlds:
                btns[0] = (s_page_button){
                    .str = "Load World",
                    .type = ek_page_button_type_load_world
                };

                btns[1] = (s_page_button){
                    .str = "Generate a New World",
                    .type = ek_page_button_type_new_world
                };

                btns[2] = (s_page_button){
                    .str = "Back",
                    .type = ek_page_button_type_redirect,
                    .redir_page = ek_title_screen_page_home
                };

                break;

            case ek_title_screen_page_settings:
                btns[0] = (s_page_button){
                    .str = "Back",
                    .type = ek_page_button_type_redirect,
                    .redir_page = ek_title_screen_page_home
                };

                break;
        }
    }

    return btns;
}

static s_vec_2d PageButtonPos(const int btn_index, const e_title_screen_page page, const s_vec_2d_i ui_size) {
    assert(ui_size.x > 0 && ui_size.y > 0);

    const int btn_cnt = g_page_button_cnts[page];

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
    ts->page_btn_hovered_index = -1;
    return true;
}

s_title_screen_tick_result TitleScreenTick(s_title_screen* const ts, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    s_title_screen_tick_result result = {0};

    const s_vec_2d_i ui_size = UISize(display_size);
    const s_vec_2d cursor_ui_pos = DisplayToUIPos(input_state->mouse_pos);

    const s_page_button* const page_btns = PushPageButtons(temp_mem_arena, ts->page);

    if (!page_btns) {
        return (s_title_screen_tick_result){
            .type = ek_title_screen_tick_result_type_error
        };
    }

    const int page_btn_cnt = g_page_button_cnts[ts->page];

    ts->page_btn_hovered_index = -1;

    for (int i = 0; i < page_btn_cnt; i++) {
        const s_page_button* const btn = &page_btns[i];

        const s_vec_2d btn_pos = PageButtonPos(i, ts->page, ui_size);

        s_rect btn_str_collider = {0};

        if (!LoadStrCollider(&btn_str_collider, btn->str, BUTTON_FONT, fonts, btn_pos, ek_str_hor_align_center, ek_str_ver_align_center, temp_mem_arena)) {
            return (s_title_screen_tick_result){
                .type = ek_title_screen_tick_result_type_error
            };
        }

        if (IsPointInRect(cursor_ui_pos, btn_str_collider)) {
            ts->page_btn_hovered_index = i;

            if (IsMouseButtonPressed(ek_mouse_button_code_left, input_state, input_state_last)) {
                switch (btn->type) {
                    case ek_page_button_type_redirect:
                        ts->page = btn->redir_page;
                        break;

                    case ek_page_button_type_exit:
                        result.world_filename = "sample.world";
                        break;

                    case ek_page_button_type_new_world:
                        if (!GenWorld("sample.world")) {
                            return (s_title_screen_tick_result){
                                .type = ek_title_screen_tick_result_type_error
                            };
                        }

                        break;

                    case ek_page_button_type_load_world:
                        result = (s_title_screen_tick_result){
                            .type = ek_title_screen_tick_result_type_load_world,
                            .world_filename = "sample.world"
                        };

                        break;
                }
            }

            break;
        }
    }

    return result;
}

bool RenderTitleScreen(const s_rendering_context* const rendering_context, const s_title_screen* const ts, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_vec_2d_i ui_size = UISize(rendering_context->display_size);

    const s_page_button* const page_btns = PushPageButtons(temp_mem_arena, ts->page);

    if (!page_btns) {
        return false;
    }

    const int page_btn_cnt = g_page_button_cnts[ts->page];

    for (int i = 0; i < page_btn_cnt; i++) {
        const s_page_button* const btn = &page_btns[i];
        const s_vec_2d btn_pos = PageButtonPos(i, ts->page, ui_size);
        const s_color btn_color = ts->page_btn_hovered_index == i ? YELLOW : WHITE;

        if (!RenderStr(rendering_context, btn->str, BUTTON_FONT, fonts, btn_pos, ek_str_hor_align_center, ek_str_ver_align_center, btn_color, temp_mem_arena)) {
            return false;
        }
    }

    return true;
}

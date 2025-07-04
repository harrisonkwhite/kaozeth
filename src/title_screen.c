#include <dirent.h>
#include "game.h"

#define BUTTON_FONT ek_font_eb_garamond_28
#define BUTTON_GAP 64.0f

#include <stdio.h>

static s_world_filenames PushWorldFilenamesToMemArena(s_mem_arena* const mem_arena, const char* const dir_name) {
    // TODO: Make this cross-platform!

    // Push the filenames to the memory arena.
    DIR* const dir = opendir(dir_name);

    if (!dir) {
        return (s_world_filenames){0};
    }

    s_world_filenames filenames = {0};

    const struct dirent* dir_entry;

    while ((dir_entry = readdir(dir)) != NULL) {
        const char* const name = dir_entry->d_name;

        if (DoesFilenameHaveExt(name, WORLD_FILENAME_EXT)) {
            const int name_len = strlen(name);

            if (name_len >= WORLD_FILENAME_BUF_SIZE) {
                continue;
            }

            t_world_filename* const ptr = MEM_ARENA_PUSH_TYPE(mem_arena, t_world_filename);

            if (!ptr) {
                closedir(dir);
                return (s_world_filenames){0};
            }

            memcpy(ptr, name, name_len);

            if (!filenames.buf) {
                // This is the first valid world filename, so have the buffer pointer point to it.
                filenames.buf = ptr;
            }

            filenames.cnt++;
        }
    }

    closedir(dir);

    return filenames;
}

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

typedef struct {
    s_page_button* buf;
    int len;
} s_page_buttons;

static int PageButtonCnt(const e_title_screen_page page, const s_world_filenames* const world_filenames) {
    assert(world_filenames);

    switch (page) {
        case ek_title_screen_page_home: return 3;
        case ek_title_screen_page_worlds: return world_filenames->cnt + 2;
        case ek_title_screen_page_settings: return 1;
    }
}

static s_page_button* PageButton(s_page_buttons* const btns, const int index) {
    assert(btns);
    assert(index >= 0 && index < btns->len);

    return &btns->buf[index];
}

static const s_page_button* PageButtonConst(const s_page_buttons* const btns, const int index) {
    assert(btns);
    assert(index >= 0 && index < btns->len);

    return &btns->buf[index];
}

static s_page_buttons PushPageButtons(s_mem_arena* const mem_arena, const e_title_screen_page page, const s_world_filenames* const world_filenames) {
    AssertMemArenaValidity(mem_arena);

    s_page_buttons btns = {
        .len = PageButtonCnt(page, world_filenames)
    };

    btns.buf = MEM_ARENA_PUSH_TYPE_MANY(mem_arena, s_page_button, btns.len);

    if (!btns.buf) {
        return (s_page_buttons){0};
    }

    switch (page) {
        case ek_title_screen_page_home:
            *PageButton(&btns, 0) = (s_page_button){
                .str = "Play",
                .type = ek_page_button_type_redirect,
                .redir_page = ek_title_screen_page_worlds
            };

            *PageButton(&btns, 1) = (s_page_button){
                .str = "Settings",
                .type = ek_page_button_type_redirect,
                .redir_page = ek_title_screen_page_settings
            };

            *PageButton(&btns, 2) = (s_page_button){
                .str = "Exit",
                .type = ek_page_button_type_exit
            };

            break;

        case ek_title_screen_page_worlds:
            for (int i = 0; i < world_filenames->cnt; i++) {
                *PageButton(&btns, i) = (s_page_button){
                    .str = world_filenames->buf[i],
                    .type = ek_page_button_type_load_world
                };
            }

            *PageButton(&btns, world_filenames->cnt) = (s_page_button){
                .str = "Generate a New World",
                .type = ek_page_button_type_new_world
            };

            *PageButton(&btns, world_filenames->cnt + 1) = (s_page_button){
                .str = "Back",
                .type = ek_page_button_type_redirect,
                .redir_page = ek_title_screen_page_home
            };

            break;

        case ek_title_screen_page_settings:
            *PageButton(&btns, 0) = (s_page_button){
                .str = "Back",
                .type = ek_page_button_type_redirect,
                .redir_page = ek_title_screen_page_home
            };

            break;
    }

    return btns;
}

static s_vec_2d PageButtonPos(const int btn_index, const int btn_cnt, const e_title_screen_page page, const s_vec_2d_i ui_size) {
    assert(ui_size.x > 0 && ui_size.y > 0);
    assert(btn_index >= 0 && btn_index < btn_cnt);

    const float btn_ver_span = (btn_cnt - 1) * BUTTON_GAP;
    const float btns_top = (ui_size.y - btn_ver_span) / 2.0f;
    return (s_vec_2d){
        ui_size.x / 2.0f,
        btns_top + (BUTTON_GAP * btn_index)
    };
}

bool InitTitleScreen(s_title_screen* const ts, s_mem_arena* const perm_mem_arena) {
    assert(IS_ZERO(*ts));

    ts->page_btn_hovered_index = -1;

    ts->world_filenames_cache = PushWorldFilenamesToMemArena(perm_mem_arena, ".");

    /* TODO: It being zero is not an error case! Distinguish!
    if (IS_ZERO(ts->world_filenames_cache)) {
        return false;
    }*/

    return true;
}

s_title_screen_tick_result TitleScreenTick(s_title_screen* const ts, const s_input_state* const input_state, const s_input_state* const input_state_last, const s_vec_2d_i display_size, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    s_title_screen_tick_result result = {0};

    const s_vec_2d_i ui_size = UISize(display_size);
    const s_vec_2d cursor_ui_pos = DisplayToUIPos(input_state->mouse_pos);

    const s_page_buttons page_btns = PushPageButtons(temp_mem_arena, ts->page, &ts->world_filenames_cache);

    if (IS_ZERO(page_btns)) {
        return (s_title_screen_tick_result){
            .type = ek_title_screen_tick_result_type_error
        };
    }

    ts->page_btn_hovered_index = -1;

    for (int i = 0; i < page_btns.len; i++) {
        const s_page_button* const btn = PageButtonConst(&page_btns, i);

        const s_vec_2d btn_pos = PageButtonPos(i, page_btns.len, ts->page, ui_size);

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
                        break;

                    case ek_page_button_type_new_world:
                        if (!GenWorld("sample.wrld")) {
                            return (s_title_screen_tick_result){
                                .type = ek_title_screen_tick_result_type_error
                            };
                        }

                        break;

                    case ek_page_button_type_load_world:
                        result = (s_title_screen_tick_result){
                            .type = ek_title_screen_tick_result_type_load_world,
                            .world_filename = "sample.wrld"
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

    const s_page_buttons page_btns = PushPageButtons(temp_mem_arena, ts->page, &ts->world_filenames_cache);

    if (IS_ZERO(page_btns)) {
        return false;
    }

    for (int i = 0; i < page_btns.len; i++) {
        const s_page_button* const btn = PageButtonConst(&page_btns, i);
        const s_vec_2d btn_pos = PageButtonPos(i, page_btns.len, ts->page, ui_size);
        const s_color btn_color = ts->page_btn_hovered_index == i ? YELLOW : WHITE;

        if (!RenderStr(rendering_context, btn->str, BUTTON_FONT, fonts, btn_pos, ek_str_hor_align_center, ek_str_ver_align_center, btn_color, temp_mem_arena)) {
            return false;
        }
    }

    return true;
}

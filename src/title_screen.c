#include <dirent.h>
#include "game.h"

#define BUTTON_GAP 64.0f

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

#if 0
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
} s_button;

typedef struct {
    s_button* buf;
    int len;
} s_buttons;
#endif

static int GetButtonCnt(const e_title_screen_page page, const s_world_filenames* const world_filenames) {
    assert(world_filenames);

    switch (page) {
        case ek_title_screen_page_home: return 3;
        case ek_title_screen_page_worlds: return world_filenames->cnt + 2;
        case ek_title_screen_page_settings: return 1;

        default:
            assert(false && "Invalid page!");
            return 0;
    }
}

typedef struct {
    s_title_screen* ts;
    s_title_screen_tick_result* tick_result;
} s_button_click_data;

static bool HomePagePlayButtonClick(const int index, void* const data_generic) {
    const s_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_worlds;
    return true;
}

static bool HomePageSettingsButtonClick(const int index, void* const data_generic) {
    const s_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_settings;
    return true;
}

static bool WorldsPageWorldButtonClick(const int index, void* const data_generic) {
    const s_button_click_data* const data = data_generic;

    assert(index < data->ts->world_filenames_cache.cnt);

    *data->tick_result = (s_title_screen_tick_result){
        .type = ek_title_screen_tick_result_type_load_world,
        .world_filename = data->ts->world_filenames_cache.buf[index]
    };

    return true;
}

static bool WorldsPageBackButtonClick(const int index, void* const data_generic) {
    const s_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_home;
    return true;
}

static bool SettingsPageBackButtonClick(const int index, void* const data_generic) {
    const s_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_home;
    return true;
}

static s_buttons PushGetButtons(s_mem_arena* const mem_arena, const e_title_screen_page page, const s_vec_2d_i ui_size, const s_world_filenames* const world_filenames) {
    AssertMemArenaValidity(mem_arena);

    s_buttons btns = {
        .cnt = GetButtonCnt(page, world_filenames)
    };

    btns.buf = MEM_ARENA_PUSH_TYPE_MANY(mem_arena, s_button, btns.cnt);

    if (!btns.buf) {
        return (s_buttons){0};
    }

    const s_vec_2d ui_mid = {
        ui_size.x / 2.0f,
        ui_size.y / 2.0f
    };

    switch (page) {
        case ek_title_screen_page_home:
            *GetButton(&btns, 0) = (s_button){
                .str = "Play",
                .pos = (s_vec_2d){ui_mid.x, ui_mid.y},
                .click_func = HomePagePlayButtonClick
            };

            *GetButton(&btns, 1) = (s_button){
                .str = "Settings",
                .pos = (s_vec_2d){ui_mid.x, ui_mid.y + (BUTTON_GAP * 1)},
                .click_func = HomePageSettingsButtonClick
            };

            *GetButton(&btns, 2) = (s_button){
                .str = "Exit",
                .pos = (s_vec_2d){ui_mid.x, ui_mid.y + (BUTTON_GAP * 2)}
            };

            break;

        case ek_title_screen_page_worlds:
            for (int i = 0; i < world_filenames->cnt; i++) {
                *GetButton(&btns, i) = (s_button){
                    .str = world_filenames->buf[i],
                    .pos = {ui_mid.x, ui_mid.y},
                    .click_func = WorldsPageWorldButtonClick
                };
            }

            *GetButton(&btns, world_filenames->cnt) = (s_button){
                .str = "Generate a New World",
                .pos = {ui_mid.x, ui_mid.y + BUTTON_GAP},
            };

            *GetButton(&btns, world_filenames->cnt + 1) = (s_button){
                .str = "Back",
                .pos = {ui_mid.x, ui_mid.y + (BUTTON_GAP * 1.75f)},
                .click_func = WorldsPageBackButtonClick
            };

            break;

        case ek_title_screen_page_settings:
            *GetButton(&btns, 0) = (s_button){
                .str = "Back",
                .pos = ui_mid,
                .click_func = SettingsPageBackButtonClick
            };

            break;

        default:
            assert(false && "Invalid page!");
            return (s_buttons){0};
    }

    // Assert that every button was set.
    for (int i = 0; i < btns.cnt; i++) {
        const s_button* const btn = &btns.buf[i];
        assert(!IS_ZERO(*btn) && "A page button wasn't set!");
    }

    return btns;
}

#if 0
static s_vec_2d GetButtonPos(const int btn_index, const int btn_cnt, const e_title_screen_page page, const s_vec_2d_i ui_size) {
    assert(ui_size.x > 0 && ui_size.y > 0);
    assert(btn_index >= 0 && btn_index < btn_cnt);

    const float btn_ver_span = (btn_cnt - 1) * BUTTON_GAP;
    const float btns_top = (ui_size.y * 0.55f) - (btn_ver_span / 2.0f);

    return (s_vec_2d){
        ui_size.x / 2.0f,
        btns_top + (BUTTON_GAP * btn_index)
    };
}
#endif

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

    const s_buttons page_btns = PushGetButtons(temp_mem_arena, ts->page, ui_size, &ts->world_filenames_cache);

    if (IS_ZERO(page_btns)) {
        return (s_title_screen_tick_result){
            .type = ek_title_screen_tick_result_type_error
        };
    }

    const s_vec_2d cursor_ui_pos = DisplayToUIPos(input_state->mouse_pos);

    if (!LoadIndexOfFirstButtonContainingPoint(&ts->page_btn_hovered_index, &page_btns, cursor_ui_pos, fonts, temp_mem_arena)) {
        return (s_title_screen_tick_result){
            .type = ek_title_screen_tick_result_type_error
        };
    }

    if (ts->page_btn_hovered_index != -1) {
        if (IsMouseButtonPressed(ek_mouse_button_code_left, input_state, input_state_last)) {
            s_button* const btn = GetButton(&page_btns, ts->page_btn_hovered_index);

            if (btn->click_func) {
                const s_button_click_data btn_click_data = {
                    .ts = ts,
                    .tick_result = &result
                };

                if (!btn->click_func(ts->page_btn_hovered_index, &btn_click_data)) {
                    return (s_title_screen_tick_result){
                        .type = ek_title_screen_tick_result_type_error
                    };
                }
            } else {
                assert(false && "Button click function not set!");
            }
#if 0
            switch (ts->page) {
                case ek_title_screen_page_home:
                    switch (ts->page_btn_hovered_index) {
                        case 0:
                            ts->page = ek_title_screen_page_worlds;
                            break;

                        case 1:
                            ts->page = ek_title_screen_page_settings;
                            break; 

                        default:
                            assert(false && "Click event for button not handled!");
                            break;
                    }

                    break;

                case ek_title_screen_page_worlds:
                    if (ts->page_btn_hovered_index)

                    for (int i = 0; i < ts->world_filenames_cache.cnt; i++) {
                        
                    }

                    switch (ts->page_btn_hovered_index) {
                        default:
                            assert(false && "Click event for button not handled!");
                            break;
                    }

                    break;

                case ek_title_screen_page_settings:
                    switch (ts->page_btn_hovered_index) {
                        case 0:
                            ts->page = ek_title_screen_page_home;
                            break;

                        default:
                            assert(false && "Click event for button not handled!");
                            break;
                    }

                    break;
            }
#endif
        }
    }

    return result;
}

bool RenderTitleScreen(const s_rendering_context* const rendering_context, const s_title_screen* const ts, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_vec_2d_i ui_size = UISize(rendering_context->display_size);

    const s_buttons page_btns = PushGetButtons(temp_mem_arena, ts->page, ui_size, &ts->world_filenames_cache);

    if (IS_ZERO(page_btns)) {
        return false;
    }

    for (int i = 0; i < page_btns.cnt; i++) {
        const s_button* const btn = GetButton(&page_btns, i);

        if (!RenderButton(rendering_context, btn, ts->page_btn_hovered_index == i, fonts, temp_mem_arena)) {
            return false;
        }
    }

    /*if (!RenderStr(rendering_context, "Select a World", ek_font_eb_garamond_48, fonts, , ek_str_hor_align_center, ek_str_ver_align_center, WHITE, temp_mem_arena)) {
        return false;
    }*/

    // Render logo.
    const s_vec_2d logo_pos = {
        ui_size.x / 2.0f,
        ui_size.y * 0.25f
    };

    if (!RenderStr(rendering_context, GAME_TITLE, ek_font_eb_garamond_80, fonts, logo_pos, ek_str_hor_align_center, ek_str_ver_align_center, WHITE, temp_mem_arena)) {
        return false;
    } 

    return true;
}

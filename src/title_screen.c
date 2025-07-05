#include <dirent.h>
#include <stdio.h>
#include "game.h"
#include "zfw_game.h"
#include "zfw_rendering.h"

#define BUTTON_GAP 64.0f
#define BUTTON_GAP_SMALL 48.0f
#define BUTTON_GAP_LARGE 80.0f

const int g_page_button_cnts[] = {
    [ek_title_screen_page_home] = 3,
    [ek_title_screen_page_worlds] = WORLD_LIMIT + 2,
    [ek_title_screen_page_new_world] = 2,
    [ek_title_screen_page_settings] = 1
};

static int WorldCnt(const t_world_filenames* const filenames) {
    int cnt = 0;

    for (int i = 0; i < WORLD_LIMIT; i++) {
        if ((*filenames)[i][0]) {
            cnt++;
        }
    }

    return cnt;
}

// TODO: Problem with this approach. If you insert an enum element between any existing one, it just zeroes out.
static_assert(STATIC_ARRAY_LEN(g_page_button_cnts) == eks_title_screen_page_cnt, "Invalid array length!");

static bool LoadWorldFilenames(t_world_filenames* const filenames, const char* const dir_name) {
    // TODO: Make this cross-platform!

    assert(filenames && IS_ZERO(*filenames));
    assert(dir_name);

    DIR* const dir = opendir(dir_name);

    if (!dir) {
        fprintf(stderr, "Failed to open directory \"%s\"!\n", dir_name);
        return false;
    }

    int cnt = 0;

    const struct dirent* dir_entry;

    while ((dir_entry = readdir(dir)) != NULL && cnt < WORLD_LIMIT) {
        const char* const name = dir_entry->d_name;

        if (DoesFilenameHaveExt(name, WORLD_FILENAME_EXT)) {
            const int name_len = strlen(name);

            if (name_len >= WORLD_FILENAME_BUF_SIZE) {
                continue;
            }

            memcpy((*filenames)[cnt], name, name_len);
            cnt++;
        }
    }

    closedir(dir);

    return true;
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

static bool HomePageExitButtonClick(const int index, void* const data_generic) {
    const s_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_worlds;
    return true;
}

static bool WorldsPageWorldButtonClick(const int index, void* const data_generic) {
    const s_button_click_data* const data = data_generic;

    assert(index < WORLD_LIMIT);
    assert(data->ts->world_filenames_cache[index][0]);

    *data->tick_result = (s_title_screen_tick_result){
        .type = ek_title_screen_tick_result_type_load_world
    };

    memcpy(data->tick_result->world_filename, data->ts->world_filenames_cache[index], sizeof(data->ts->world_filenames_cache[index]));

    return true;
}

static bool WorldsPageGenButtonClick(const int index, void* const data_generic) {
    const s_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_new_world;
    assert(IS_ZERO(data->ts->new_world_name_buf));
    return true;
}

static bool WorldsPageBackButtonClick(const int index, void* const data_generic) {
    const s_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_home;
    return true;
}

static bool NewWorldPageAcceptButtonClick(const int index, void* const data_generic) {
    const s_button_click_data* const data = data_generic;

    data->ts->page = ek_title_screen_page_worlds;

    t_world_filename filename = {0};
    snprintf(filename, sizeof(filename), "%s%s", data->ts->new_world_name_buf, WORLD_FILENAME_EXT);

    if (!GenWorld(&filename)) {
        return false;
    }

    ZERO_OUT(data->ts->new_world_name_buf);

    ZERO_OUT(data->ts->world_filenames_cache);
    return LoadWorldFilenames(&data->ts->world_filenames_cache, ".");
}

static bool NewWorldPageBackButtonClick(const int index, void* const data_generic) {
    const s_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_worlds;
    ZERO_OUT(data->ts->new_world_name_buf);
    return true;
}

static bool SettingsPageBackButtonClick(const int index, void* const data_generic) {
    const s_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_home;
    return true;
}

static s_buttons PushPageButtons(s_mem_arena* const mem_arena, const e_title_screen_page page, const s_vec_2d_i ui_size, const t_world_filenames* const world_filenames, const t_world_name_buf* const new_world_name_buf) {
    AssertMemArenaValidity(mem_arena);

    const s_buttons btns = {
        .buf = MEM_ARENA_PUSH_TYPE_MANY(mem_arena, s_button, g_page_button_cnts[page]),
        .cnt = g_page_button_cnts[page]
    };

    if (!btns.buf) {
        return (s_buttons){0};
    }

    const s_vec_2d ui_mid = {
        ui_size.x / 2.0f,
        ui_size.y / 2.0f
    };

    float y_pen = 0.0f;

    switch (page) {
        case ek_title_screen_page_home:
            for (int i = 0; i < btns.cnt; i++) {
                s_button* const btn = GetButtonConst(&btns, i);

                switch (i) {
                    case 0:
                        *btn = (s_button){
                            .str = "Play",
                            .click_func = HomePagePlayButtonClick
                        };

                        break;

                    case 1:
                        *btn = (s_button){
                            .str = "Settings",
                            .click_func = HomePageSettingsButtonClick
                        };

                        break;

                    case 2:
                        *btn = (s_button){
                            .str = "Exit",
                            .click_func = HomePageExitButtonClick
                        };

                        break;

                    default:
                        assert(false && "Unhandled button case!");
                        break;
                }

                btn->pos = (s_vec_2d){ui_mid.x, y_pen};

                if (i < btns.cnt - 1) {
                    y_pen += BUTTON_GAP;
                }
            }

            break;

        case ek_title_screen_page_worlds:
            for (int i = 0; i < WORLD_LIMIT; i++) {
                s_button* const btn = GetButtonConst(&btns, i);

                *btn = (s_button){
                    .pos = {ui_mid.x, y_pen},
                    .click_func = WorldsPageWorldButtonClick
                };

                if (i < WORLD_LIMIT - 1) {
                    y_pen += BUTTON_GAP_SMALL;
                }

                if (!(*world_filenames)[i][0]) {
                    snprintf(btn->str, sizeof(btn->str), "Empty");
                    btn->inactive = true;
                } else {
                    snprintf(btn->str, sizeof(btn->str), "%s", (*world_filenames)[i]);
                }
            }

            y_pen += BUTTON_GAP_LARGE;

            *GetButtonConst(&btns, WORLD_LIMIT) = (s_button){
                .str = "Create New World",
                .pos = {ui_mid.x, y_pen},
                .click_func = WorldsPageGenButtonClick,
                .inactive = WorldCnt(world_filenames) == WORLD_LIMIT
            };

            y_pen += BUTTON_GAP_SMALL;

            *GetButtonConst(&btns, WORLD_LIMIT + 1) = (s_button){
                .str = "Back",
                .pos = {ui_mid.x, y_pen},
                .click_func = WorldsPageBackButtonClick
            };

            break;

        case ek_title_screen_page_new_world:
            y_pen = BUTTON_GAP_SMALL + BUTTON_GAP_LARGE;

            for (int i = 0; i < btns.cnt; i++) {
                s_button* const btn = GetButtonConst(&btns, i);

                switch (i) {
                    case 0:
                        *btn = (s_button){
                            .str = "Accept",
                            .click_func = NewWorldPageAcceptButtonClick,
                            .inactive = !(*new_world_name_buf)[0]
                        };

                        break;

                    case 1:
                        *btn = (s_button){
                            .str = "Back",
                            .click_func = NewWorldPageBackButtonClick
                        };

                        break;

                    default:
                        assert(false && "Unhandled button case!");
                        break;
                }

                btn->pos = (s_vec_2d){ui_mid.x, y_pen};

                if (i < btns.cnt - 1) {
                    y_pen += BUTTON_GAP_SMALL;
                }
            }

            break;

        case ek_title_screen_page_settings:
            *GetButtonConst(&btns, 0) = (s_button){
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

    // Vertically center the buttons.
    const float y_span = y_pen;

    for (int i = 0; i < btns.cnt; i++) {
        s_button* const btn = &btns.buf[i];
        btn->pos.y += ui_mid.y;//ui_mid.y - (y_span / 2.0f);
    }

    return btns;
}

bool InitTitleScreen(s_title_screen* const ts) {
    assert(IS_ZERO(*ts));

    ts->page_btn_hovered_index = -1;

    if (!LoadWorldFilenames(&ts->world_filenames_cache, ".")) {
        return false;
    }

    return true;
}

s_title_screen_tick_result TitleScreenTick(s_title_screen* const ts, const s_input_state* const input_state, const s_input_state* const input_state_last, const t_unicode_buf* const unicode_buf, const s_vec_2d_i display_size, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    s_title_screen_tick_result result = {0};

    const s_vec_2d_i ui_size = UISize(display_size);

    const s_buttons page_btns = PushPageButtons(temp_mem_arena, ts->page, ui_size, &ts->world_filenames_cache, &ts->new_world_name_buf);

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
            s_button* const btn = GetButtonConst(&page_btns, ts->page_btn_hovered_index);

            if (btn->click_func) {
                s_button_click_data btn_click_data = {
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
        }
    }

    if (ts->page == ek_title_screen_page_new_world) {
        int nw_name_buf_index = 0;

        while (nw_name_buf_index < sizeof(ts->new_world_name_buf) && ts->new_world_name_buf[nw_name_buf_index]) {
            nw_name_buf_index++;
        }

        for (int i = 0; i < sizeof(*unicode_buf) && nw_name_buf_index < sizeof(ts->new_world_name_buf) - 1; i++) {
            const char c = (*unicode_buf)[i];

            if (!c) {
                break;
            }

            ts->new_world_name_buf[nw_name_buf_index] = c;
            nw_name_buf_index++;
        }

        if (nw_name_buf_index > 0) {
            if (IsKeyPressed(ek_key_code_backspace, input_state, input_state_last)) {
                nw_name_buf_index--;
                ts->new_world_name_buf[nw_name_buf_index] = '\0';
            }
        }
    }

    return result;
}

bool RenderTitleScreen(const s_rendering_context* const rendering_context, const s_title_screen* const ts, const s_textures* const textures, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_vec_2d_i ui_size = UISize(rendering_context->display_size);

    const s_buttons page_btns = PushPageButtons(temp_mem_arena, ts->page, ui_size, &ts->world_filenames_cache, &ts->new_world_name_buf);

    if (IS_ZERO(page_btns)) {
        return false;
    }

    for (int i = 0; i < page_btns.cnt; i++) {
        const s_button* const btn = GetButtonConst(&page_btns, i);

        if (!RenderButton(rendering_context, btn, ts->page_btn_hovered_index == i, fonts, temp_mem_arena)) {
            return false;
        }
    }

    if (ts->page == ek_title_screen_page_new_world) {
        const s_vec_2d top_pos = {
            ui_size.x / 2.0f,
            ui_size.y / 2.0f
        };

        if (!RenderStr(rendering_context, "Enter world name:", ek_font_eb_garamond_32, fonts, top_pos, ek_str_hor_align_center, ek_str_ver_align_center, WHITE, temp_mem_arena)) {
            return false;
        }

        if (ts->new_world_name_buf[0]) {
            const s_vec_2d dog_pos = {
                ui_size.x / 2.0f,
                (ui_size.y / 2.0f) + (BUTTON_GAP * 0.75f)
            };

            if (!RenderStr(rendering_context, ts->new_world_name_buf, ek_font_eb_garamond_32, fonts, dog_pos, ek_str_hor_align_center, ek_str_ver_align_center, WHITE, temp_mem_arena)) {
                return false;
            }
        }
    }

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

#include <stdio.h>
#include <dirent.h>
#include <zfw_game.h>
#include <zfw_rendering.h>
#include "game.h"

#define PAGE_ELEM_COMMON_FONT ek_font_eb_garamond_32
#define PAGE_ELEM_GAP 48.0f
#define PAGE_ELEM_COMMON_PADDING 28.0f

typedef struct {
    s_title_screen* ts;
    s_title_screen_tick_result* tick_result;
} s_page_elem_button_click_data;

typedef bool (*t_page_elem_button_click_func)(const int index, void* const data);

typedef struct {
    char str[32];
    float padding_top;
    float padding_bottom;
    e_font font;

    bool button;
    bool button_inactive;
    t_page_elem_button_click_func button_click_func;
} s_page_elem;

typedef struct {
    const s_page_elem* buf;
    int cnt;
} s_page_elems;

static bool LoadWorldFilenames(t_world_filenames* const filenames, const char* const dir_name) {
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

static int WorldCnt(const t_world_filenames* const filenames) {
    int cnt = 0;

    for (int i = 0; i < WORLD_LIMIT; i++) {
        if ((*filenames)[i][0]) {
            cnt++;
        }
    }

    return cnt;
}

static bool HomePagePlayButtonClick(const int index, void* const data_generic) {
    const s_page_elem_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_worlds;
    return true;
}

static bool HomePageSettingsButtonClick(const int index, void* const data_generic) {
    const s_page_elem_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_settings;
    return true;
}

static bool HomePageExitButtonClick(const int index, void* const data_generic) {
    const s_page_elem_button_click_data* const data = data_generic;

    *data->tick_result = (s_title_screen_tick_result){
        .type = ek_title_screen_tick_result_type_exit
    };

    return true;
}

static bool WorldsPageWorldButtonClick(const int index, void* const data_generic) {
    const s_page_elem_button_click_data* const data = data_generic;

    const int world_index = index - 1;
    assert(world_index >= 0 && world_index < WORLD_LIMIT);
    assert(data->ts->world_filenames_cache[world_index][0]);

    *data->tick_result = (s_title_screen_tick_result){
        .type = ek_title_screen_tick_result_type_load_world
    };

    memcpy(data->tick_result->world_filename, data->ts->world_filenames_cache[world_index], sizeof(data->ts->world_filenames_cache[world_index]));

    return true;
}

static bool WorldsPageCreateButtonClick(const int index, void* const data_generic) {
    const s_page_elem_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_new_world;
    assert(IS_ZERO(data->ts->new_world_name_buf));
    return true;
}

static bool WorldsPageBackButtonClick(const int index, void* const data_generic) {
    const s_page_elem_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_home;
    return true;
}

static bool NewWorldPageAcceptButtonClick(const int index, void* const data_generic) {
    const s_page_elem_button_click_data* const data = data_generic;

    data->ts->page = ek_title_screen_page_worlds;

    t_world_filename filename = {0};
    snprintf(filename, sizeof(filename), "%s%s", data->ts->new_world_name_buf, WORLD_FILENAME_EXT);

    {
        s_world_core world_core = {0};
        GenWorld(&world_core);

        if (!WriteWorldCoreToFile(&world_core, &filename)) {
            return false;
        }
    }

    ZERO_OUT(data->ts->new_world_name_buf);

    ZERO_OUT(data->ts->world_filenames_cache);
    return LoadWorldFilenames(&data->ts->world_filenames_cache, ".");
}

static bool NewWorldPageBackButtonClick(const int index, void* const data_generic) {
    const s_page_elem_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_worlds;
    ZERO_OUT(data->ts->new_world_name_buf);
    return true;
}

static bool SettingsPageBackButtonClick(const int index, void* const data_generic) {
    const s_page_elem_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_home;
    return true;
}

static s_page_elems PushPageElems(s_mem_arena* const mem_arena, const e_title_screen_page page, const t_world_filenames* const world_filenames, const t_world_name_buf* const new_world_name_buf) {
    int elem_cnt;

    switch (page) {
        case ek_title_screen_page_home: elem_cnt = 3; break;
        case ek_title_screen_page_worlds: elem_cnt = 1 + WORLD_LIMIT + 2; break;
        case ek_title_screen_page_new_world: elem_cnt = 4; break;
        case ek_title_screen_page_settings: elem_cnt = 1; break;

        default:
            assert(false && "Unhandled page case!");
            break;
    }

    s_page_elem* const elems = MEM_ARENA_PUSH_TYPE_MANY(mem_arena, s_page_elem, elem_cnt);

    if (!elems) {
        return (s_page_elems){0};
    }

    switch (page) {
        case ek_title_screen_page_home:
            for (int i = 0; i < elem_cnt; i++) {
                switch (i) {
                    case 0:
                        elems[i] = (s_page_elem){
                            .str = "Play",
                            .font = PAGE_ELEM_COMMON_FONT,
                            .padding_top = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .padding_bottom = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .button = true,
                            .button_click_func = HomePagePlayButtonClick
                        };

                        break;

                    case 1:
                        elems[i] = (s_page_elem){
                            .str = "Settings",
                            .font = PAGE_ELEM_COMMON_FONT,
                            .padding_top = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .padding_bottom = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .button = true,
                            .button_click_func = HomePageSettingsButtonClick
                        };

                        break;

                    case 2:
                        elems[i] = (s_page_elem){
                            .str = "Exit",
                            .font = PAGE_ELEM_COMMON_FONT,
                            .padding_top = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .padding_bottom = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .button = true,
                            .button_click_func = HomePageExitButtonClick
                        };

                        break;

                    default:
                        assert(false && "Unhandled element case!");
                        break;
                }
            }

            break;

        case ek_title_screen_page_worlds:
            elems[0] = (s_page_elem){
                .str = "Select a World",
                .font = ek_font_eb_garamond_48,
                .padding_bottom = PAGE_ELEM_COMMON_PADDING
            }; 

            for (int i = 0; i < WORLD_LIMIT; i++) {
                s_page_elem* const elem = &elems[1 + i];

                *elem = (s_page_elem){
                    .font = PAGE_ELEM_COMMON_FONT,
                    .button = true,
                    .button_click_func = WorldsPageWorldButtonClick
                };

                if (!(*world_filenames)[i][0]) {
                    snprintf(elem->str, sizeof(elem->str), "Empty");
                    elem->button_inactive = true;
                } else {
                    snprintf(elem->str, sizeof(elem->str), "%s", (*world_filenames)[i]);
                }
            }

            elems[1 + WORLD_LIMIT] = (s_page_elem){
                .str = "Create New World",
                .font = PAGE_ELEM_COMMON_FONT,
                .padding_top = PAGE_ELEM_COMMON_PADDING,
                .button = true,
                .button_inactive = WorldCnt(world_filenames) == WORLD_LIMIT,
                .button_click_func = WorldsPageCreateButtonClick
            };

            elems[1 + WORLD_LIMIT + 1] = (s_page_elem){
                .str = "Back",
                .font = PAGE_ELEM_COMMON_FONT,
                .button = true,
                .button_click_func = WorldsPageBackButtonClick
            };

            break;

        case ek_title_screen_page_new_world:
            elems[0] = (s_page_elem){
                .str = "Enter World Name",
                .font = ek_font_eb_garamond_48,
                .padding_bottom = PAGE_ELEM_COMMON_PADDING
            };

            elems[1] = (s_page_elem){
                .font = PAGE_ELEM_COMMON_FONT
            };

            snprintf(elems[1].str, sizeof(elems[1].str), "%s", *new_world_name_buf);

            elems[2] = (s_page_elem){
                .str = "Accept",
                .padding_top = PAGE_ELEM_COMMON_PADDING,
                .font = PAGE_ELEM_COMMON_FONT,
                .button = true,
                .button_inactive = !(*new_world_name_buf)[0],
                .button_click_func = NewWorldPageAcceptButtonClick
            };

            elems[3] = (s_page_elem){
                .str = "Back",
                .font = PAGE_ELEM_COMMON_FONT,
                .button = true,
                .button_click_func = NewWorldPageBackButtonClick
            };

            break;

        case ek_title_screen_page_settings:
            elems[0] = (s_page_elem){
                .str = "Back",
                .font = PAGE_ELEM_COMMON_FONT,
                .button = true,
                .button_click_func = SettingsPageBackButtonClick
            }; 

            break;
    }

    return (s_page_elems){
        .buf = elems,
        .cnt = elem_cnt
    };
}

static const s_vec_2d* PushPageElemPositions(s_mem_arena* const mem_arena, const s_page_elems* const elems, const s_vec_2d_i ui_size) {
    s_vec_2d* const positions = MEM_ARENA_PUSH_TYPE_MANY(mem_arena, s_vec_2d, elems->cnt);

    if (positions) {
        float span_y = 0.0f;

        for (int i = 0; i < elems->cnt; i++) {
            const s_page_elem* const elem = &elems->buf[i];

            span_y += elem->padding_top;

            positions[i] = (s_vec_2d){
                ui_size.x / 2.0f,
                span_y
            };

            span_y += elem->padding_bottom;

            if (i < elems->cnt - 1) {
                span_y += PAGE_ELEM_GAP;
            }
        }

        // Shift everything vertically to the middle.
        for (int i = 0; i < elems->cnt; i++) {
            positions[i].y += (ui_size.y - span_y) / 2.0f;
        }
    }

    return positions;
}

bool InitTitleScreen(s_title_screen* const ts) {
    assert(IS_ZERO(*ts));

    ts->page_btn_elem_hovered_index = -1;

    if (!LoadWorldFilenames(&ts->world_filenames_cache, ".")) {
        return false;
    }

    return true;
}

static bool LoadIndexOfFirstHoveredButtonPageElem(int* const index, const s_vec_2d cursor_ui_pos, const s_page_elems* const elems, const s_vec_2d* const elem_positions, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    assert(index);

    *index = -1;

    for (int i = 0; i < elems->cnt; i++) {
        const s_page_elem* const elem = &elems->buf[i];

        if (!elem->button || elem->button_inactive) {
            continue;
        }

        s_rect collider = {0};

        if (!LoadStrCollider(&collider, elem->str, elem->font, fonts, elem_positions[i], ek_str_hor_align_center, ek_str_ver_align_center, temp_mem_arena)) {
            return false;
        }

        if (IsPointInRect(cursor_ui_pos, collider)) {
            *index = i;
            break;
        }
    }

    return true;
}

s_title_screen_tick_result TitleScreenTick(s_title_screen* const ts, const s_input_state* const input_state, const s_input_state* const input_state_last, const t_unicode_buf* const unicode_buf, const s_vec_2d_i display_size, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    s_title_screen_tick_result result = {0};

    const s_page_elems page_elems = PushPageElems(temp_mem_arena, ts->page, &ts->world_filenames_cache, &ts->new_world_name_buf);

    if (IS_ZERO(page_elems)) {
        return (s_title_screen_tick_result){
            ek_title_screen_tick_result_type_error
        };
    }

    const s_vec_2d_i ui_size = UISize(display_size);
    const s_vec_2d* const elem_positions = PushPageElemPositions(temp_mem_arena, &page_elems, ui_size);

    if (IS_ZERO(page_elems)) {
        return (s_title_screen_tick_result){
            ek_title_screen_tick_result_type_error
        };
    }

    const s_vec_2d cursor_ui_pos = DisplayToUIPos(input_state->mouse_pos);

    if (!LoadIndexOfFirstHoveredButtonPageElem(&ts->page_btn_elem_hovered_index, cursor_ui_pos, &page_elems, elem_positions, fonts, temp_mem_arena)) {
        return (s_title_screen_tick_result){
            ek_title_screen_tick_result_type_error
        };
    }

    if (ts->page_btn_elem_hovered_index != -1) {
        if (IsMouseButtonPressed(ek_mouse_button_code_left, input_state, input_state_last)) {
            const s_page_elem* const elem = &page_elems.buf[ts->page_btn_elem_hovered_index];

            if (elem->button_click_func) {
                s_page_elem_button_click_data btn_click_data = {
                    .ts = ts,
                    .tick_result = &result
                };

                if (!elem->button_click_func(ts->page_btn_elem_hovered_index, &btn_click_data)) {
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
    const s_page_elems page_elems = PushPageElems(temp_mem_arena, ts->page, &ts->world_filenames_cache, &ts->new_world_name_buf);

    if (IS_ZERO(page_elems)) {
        return false;
    }

    const s_vec_2d* const positions = PushPageElemPositions(temp_mem_arena, &page_elems, UISize(rendering_context->display_size));

    if (!positions) {
        return false;
    }

    for (int i = 0; i < page_elems.cnt; i++) {
        const s_page_elem* const elem = &page_elems.buf[i];

        s_color color = WHITE;

        if (elem->button) {
            if (elem->button_inactive) {
                color = GRAY;
            } else if (ts->page_btn_elem_hovered_index == i) {
                color = YELLOW;
            }
        }

        if (!RenderStr(rendering_context, elem->str, elem->font, fonts, positions[i], ek_str_hor_align_center, ek_str_ver_align_center, color, temp_mem_arena)) {
            return false;
        }
    }

    return true;
}

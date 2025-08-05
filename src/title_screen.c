#include "game.h"

#include <stdio.h>

#define PAGE_ELEM_COMMON_FONT ek_font_eb_garamond_32
#define PAGE_ELEM_GAP 48.0f
#define PAGE_ELEM_COMMON_PADDING 28.0f
#define PAGE_ELEM_STR_BUF_SIZE 32
static_assert(WORLD_NAME_LEN_LIMIT <= PAGE_ELEM_STR_BUF_SIZE - 1, "A page element must be able to represent a maximum-length world name!");

typedef struct {
    s_title_screen* ts;
    t_settings* settings;
    s_title_screen_tick_result* tick_result;
    s_mem_arena* temp_mem_arena;
} s_page_elem_button_click_data;

typedef bool (*t_page_elem_button_click_func)(const int index, void* const data);

typedef struct {
    char str[PAGE_ELEM_STR_BUF_SIZE];
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

static bool LoadWorldFilenames(t_world_filenames* const filenames, s_mem_arena* const temp_mem_arena) {
    assert(filenames && IS_ZERO(*filenames));
    assert(temp_mem_arena && IsMemArenaValid(temp_mem_arena));

    s_filenames local_filenames = {0};

    if (!LoadDirectoryFilenames(&local_filenames, temp_mem_arena, ".")) {
        return false;
    }

    int next_index = 0;

    for (int i = 0; i < local_filenames.cnt; i++) {
        if (DoesFilenameHaveExt(local_filenames.buf[i], WORLD_FILENAME_EXT)) {
            // TODO: Make sure the world filename isn't too long!
            strncpy((*filenames)[next_index], local_filenames.buf[i], sizeof((*filenames)[next_index]));
            next_index++;

            if (next_index == WORLD_LIMIT) {
                break;
            }
        }
    }

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
    return LoadWorldFilenames(&data->ts->world_filenames_cache, data->temp_mem_arena);
}

static bool NewWorldPageBackButtonClick(const int index, void* const data_generic) {
    const s_page_elem_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_worlds;
    ZERO_OUT(data->ts->new_world_name_buf);
    return true;
}

static bool SettingsPageSettingButtonClick(const int index, void* const data_generic) {
    const s_page_elem_button_click_data* const data = data_generic;

    assert(index >= 0 && index < eks_setting_cnt);

    int limit;
    int inc = 1;

    switch (g_settings[index].type) {
        case ek_setting_type_toggle:
            limit = 1;
            break;

        case ek_setting_type_perc:
            limit = 100;
            inc = 5;
            break;
    }

    if ((*data->settings)[index] == limit) {
        (*data->settings)[index] = 0;
    } else {
        (*data->settings)[index] += inc;
    }

    return true;
}

static bool SettingsPageBackButtonClick(const int index, void* const data_generic) {
    const s_page_elem_button_click_data* const data = data_generic;
    data->ts->page = ek_title_screen_page_home;
    return true;
}

static s_page_elems PushPageElems(s_mem_arena* const mem_arena, const e_title_screen_page page, const t_world_filenames* const world_filenames, const t_world_name_buf* const new_world_name_buf, const t_settings* const settings) {
    int elem_cnt;

    switch (page) {
        case ek_title_screen_page_home: elem_cnt = 4; break;
        case ek_title_screen_page_worlds: elem_cnt = 1 + WORLD_LIMIT + 2; break;
        case ek_title_screen_page_new_world: elem_cnt = 4; break;
        case ek_title_screen_page_settings: elem_cnt = eks_setting_cnt + 1; break;

        default:
            assert(false && "Unhandled page case!");
            break;
    }

    s_page_elem* const elems = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, s_page_elem, elem_cnt);

    if (!elems) {
        return (s_page_elems){0};
    }

    switch (page) {
        case ek_title_screen_page_home:
            for (int i = 0; i < elem_cnt; i++) {
                switch (i) {
                    case 0:
                        elems[i] = (s_page_elem){
                            .str = GAME_TITLE,
                            .font = ek_font_eb_garamond_80,
                            .padding_bottom = 48.0f
                        };

                        break;

                    case 1:
                        elems[i] = (s_page_elem){
                            .str = "Play",
                            .font = PAGE_ELEM_COMMON_FONT,
                            .padding_top = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .padding_bottom = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .button = true,
                            .button_click_func = HomePagePlayButtonClick
                        };

                        break;

                    case 2:
                        elems[i] = (s_page_elem){
                            .str = "Settings",
                            .font = PAGE_ELEM_COMMON_FONT,
                            .padding_top = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .padding_bottom = PAGE_ELEM_COMMON_PADDING / 2.0f,
                            .button = true,
                            .button_click_func = HomePageSettingsButtonClick
                        };

                        break;

                    case 3:
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
            for (int i = 0; i < eks_setting_cnt; i++) {
                elems[i] = (s_page_elem){
                    .font = PAGE_ELEM_COMMON_FONT,

                    .button = true,
                    .button_click_func = SettingsPageSettingButtonClick
                };

                switch ((e_setting_type)g_settings[i].type) {
                    case ek_setting_type_toggle:
                        if (SettingToggle(settings, i)) {
                            snprintf(elems[i].str, sizeof(elems[i].str), "%s: Enabled", g_settings[i].name);
                        } else {
                            snprintf(elems[i].str, sizeof(elems[i].str), "%s: Disabled", g_settings[i].name);
                        }

                        break;

                    case ek_setting_type_perc:
                        snprintf(elems[i].str, sizeof(elems[i].str), "%s: %d%%", g_settings[i].name, (int)(*settings)[i]);
                        break;
                }

            }

            elems[eks_setting_cnt] = (s_page_elem){
                .str = "Back",
                .font = PAGE_ELEM_COMMON_FONT,
                .padding_top = PAGE_ELEM_COMMON_PADDING,
                .button = true,
                .button_click_func = SettingsPageBackButtonClick
            };

            break;

        default:
            assert(false && "Unhandled switch case!");
            break;
    }

    return (s_page_elems){
        .buf = elems,
        .cnt = elem_cnt
    };
}

static const s_v2* PushPageElemPositions(s_mem_arena* const mem_arena, const s_page_elems* const elems, const s_v2_int ui_size) {
    s_v2* const positions = MEM_ARENA_PUSH_TYPE_CNT(mem_arena, s_v2, elems->cnt);

    if (positions) {
        float span_y = 0.0f;

        for (int i = 0; i < elems->cnt; i++) {
            const s_page_elem* const elem = &elems->buf[i];

            span_y += elem->padding_top;

            positions[i] = (s_v2){
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

bool InitTitleScreen(s_title_screen* const ts, s_mem_arena* const temp_mem_arena) {
    assert(IS_ZERO(*ts));

    ts->page_btn_elem_hovered_index = -1;

    if (!LoadWorldFilenames(&ts->world_filenames_cache, temp_mem_arena)) {
        return false;
    }

    return true;
}

static bool LoadIndexOfFirstHoveredButtonPageElem(int* const index, const s_v2 cursor_ui_pos, const s_page_elems* const elems, const s_v2* const elem_positions, const zfw_s_font_group* const fonts, s_mem_arena* const temp_mem_arena) {
    assert(index);

    *index = -1;

    for (int i = 0; i < elems->cnt; i++) {
        const s_page_elem* const elem = &elems->buf[i];

        if (!elem->button || elem->button_inactive) {
            continue;
        }

        zfw_s_rect collider = {0};

        if (!ZFW_LoadStrCollider(&collider, elem->str, fonts, elem->font, elem_positions[i], ZFW_ALIGNMENT_CENTER, temp_mem_arena)) {
            return false;
        }

        if (ZFW_IsPointInRect(cursor_ui_pos, collider)) {
            *index = i;
            break;
        }
    }

    return true;
}

s_title_screen_tick_result TitleScreenTick(s_title_screen* const ts, t_settings* const settings, const zfw_s_game_tick_context* const zfw_context, const zfw_s_font_group* const fonts, const zfw_s_sound_types* const snd_types) {
    s_title_screen_tick_result result = {0};

    const s_page_elems page_elems = PushPageElems(zfw_context->temp_mem_arena, ts->page, &ts->world_filenames_cache, &ts->new_world_name_buf, settings);

    if (IS_ZERO(page_elems)) {
        return (s_title_screen_tick_result){
            ek_title_screen_tick_result_type_error
        };
    }

    const s_v2_int ui_size = UISize(zfw_context->window_state.size);
    const s_v2* const elem_positions = PushPageElemPositions(zfw_context->temp_mem_arena, &page_elems, ui_size);

    if (IS_ZERO(page_elems)) {
        return (s_title_screen_tick_result){
            ek_title_screen_tick_result_type_error
        };
    }

    const s_v2 cursor_ui_pos = DisplayToUIPos(zfw_context->input_context.state->mouse_pos, zfw_context->window_state.size);

    if (!LoadIndexOfFirstHoveredButtonPageElem(&ts->page_btn_elem_hovered_index, cursor_ui_pos, &page_elems, elem_positions, fonts, zfw_context->temp_mem_arena)) {
        return (s_title_screen_tick_result){
            ek_title_screen_tick_result_type_error
        };
    }

    if (ts->page_btn_elem_hovered_index != -1) {
        if (ZFW_IsMouseButtonPressed(&zfw_context->input_context, zfw_ek_mouse_button_code_left)) {
            const s_page_elem* const elem = &page_elems.buf[ts->page_btn_elem_hovered_index];

            if (elem->button_click_func) {
                s_page_elem_button_click_data btn_click_data = {
                    .ts = ts,
                    .settings = settings,
                    .tick_result = &result,
                    .temp_mem_arena = zfw_context->temp_mem_arena
                };

                if (!elem->button_click_func(ts->page_btn_elem_hovered_index, &btn_click_data)) {
                    return (s_title_screen_tick_result){
                        .type = ek_title_screen_tick_result_type_error
                    };
                }
            } else {
                assert(false && "Button click function not set!");
            }

            if (!ZFW_PlaySound(zfw_context->audio_sys, snd_types, ek_sound_type_button_click, SettingPerc(settings, ek_setting_volume), ZFW_PAN_DEFAULT, ZFW_PITCH_DEFAULT)) {
                return (s_title_screen_tick_result){
                    ek_title_screen_tick_result_type_error
                };
            }
        }
    }

    if (ts->page == ek_title_screen_page_new_world) {
        int nw_name_buf_index = 0;

        while (nw_name_buf_index < sizeof(ts->new_world_name_buf) && ts->new_world_name_buf[nw_name_buf_index]) {
            nw_name_buf_index++;
        }

        for (size_t i = 0; i < sizeof(zfw_context->input_context.events->unicode_buf) && nw_name_buf_index < sizeof(ts->new_world_name_buf) - 1; i++) {
            const char c = zfw_context->input_context.events->unicode_buf[i];

            if (!c) {
                break;
            }

            ts->new_world_name_buf[nw_name_buf_index] = c;
            nw_name_buf_index++;
        }

        if (nw_name_buf_index > 0) {
            if (ZFW_IsKeyPressed(&zfw_context->input_context, zfw_ek_key_code_backspace)) {
                nw_name_buf_index--;
                ts->new_world_name_buf[nw_name_buf_index] = '\0';
            }
        }
    }

    return result;
}

bool RenderTitleScreen(const s_title_screen* const ts, const zfw_s_rendering_context* const rendering_context, const t_settings* const settings, const zfw_s_texture_group* const textures, const zfw_s_font_group* const fonts, s_mem_arena* const temp_mem_arena) {
    const s_page_elems page_elems = PushPageElems(temp_mem_arena, ts->page, &ts->world_filenames_cache, &ts->new_world_name_buf, settings);

    if (IS_ZERO(page_elems)) {
        return false;
    }

    const s_v2* const positions = PushPageElemPositions(temp_mem_arena, &page_elems, UISize(rendering_context->window_size));

    if (!positions) {
        return false;
    }

    for (int i = 0; i < page_elems.cnt; i++) {
        const s_page_elem* const elem = &page_elems.buf[i];

        if (!elem->str[0]) {
            continue;
        }

        u_v4 color = ZFW_WHITE;

        if (elem->button) {
            if (elem->button_inactive) {
                color = ZFW_GRAY;
            } else if (ts->page_btn_elem_hovered_index == i) {
                color = ZFW_YELLOW;
            }
        }

        if (!ZFW_RenderStr(rendering_context, elem->str, fonts, elem->font, positions[i], ZFW_ALIGNMENT_CENTER, color, temp_mem_arena)) {
            return false;
        }
    }

    return true;
}

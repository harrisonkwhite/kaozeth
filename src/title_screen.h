#ifndef TITLE_SCREEN_H
#define TITLE_SCREEN_H

#include <zfw.h>
#include "world/world.h"

#define WORLD_LIMIT 3

typedef t_world_filename t_world_filenames[WORLD_LIMIT];

typedef enum {
    ek_title_screen_tick_result_type_default,
    ek_title_screen_tick_result_type_error,
    ek_title_screen_tick_result_type_load_world,
    ek_title_screen_tick_result_type_exit
} e_title_screen_tick_result_type;

typedef struct {
    e_title_screen_tick_result_type type;
    t_world_filename world_filename;
} s_title_screen_tick_result;

typedef enum {
    ek_title_screen_page_home,
    ek_title_screen_page_worlds,
    ek_title_screen_page_new_world,
    ek_title_screen_page_settings,

    eks_title_screen_page_cnt
} e_title_screen_page;

typedef char t_world_name_buf[WORLD_NAME_LEN_LIMIT + 1];

typedef struct {
    e_title_screen_page page;
    int page_btn_elem_hovered_index;
    t_world_filenames world_filenames_cache;
    t_world_name_buf new_world_name_buf;
} s_title_screen;

bool InitTitleScreen(s_title_screen* const ts, zfw_s_mem_arena* const temp_mem_arena);
s_title_screen_tick_result TitleScreenTick(s_title_screen* const ts, t_settings* const settings, const zfw_s_input_state* const input_state, const zfw_s_input_state* const input_state_last, const zfw_t_unicode_buf* const unicode_buf, const zfw_s_vec_2d_i display_size, const zfw_s_fonts* const fonts, zfw_s_audio_sys* const audio_sys, const zfw_s_sound_types* const snd_types, zfw_s_mem_arena* const temp_mem_arena);
bool RenderTitleScreen(const zfw_s_rendering_context* const rendering_context, const s_title_screen* const ts, const t_settings* const settings, const zfw_s_textures* const textures, const zfw_s_fonts* const fonts, zfw_s_mem_arena* const temp_mem_arena);

#endif

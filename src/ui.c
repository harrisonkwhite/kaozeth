#include "game.h"

#define BUTTON_FONT ek_font_eb_garamond_32
#define BUTTON_COLOR WHITE
#define BUTTON_COLOR_HOVER YELLOW

static inline bool IsButtonValid(const s_button* const btn) {
    assert(btn);
    return btn->str != NULL;
}

static inline bool IsButtonsValid(const s_buttons* const btns) {
    assert(btns);

    if (IS_ZERO(*btns)) {
        return true;
    }

    if (!btns->buf || btns->cnt <= 0) {
        return false;
    }

    for (int i = 0; i < btns->cnt; i++) {
        if (!IsButtonValid(&btns->buf[i])) {
            return false;
        }
    }

    return true;
}

s_button* GetButton(s_buttons* const btns, const int index) {
    assert(btns);
    assert(index >= 0 && index < btns->cnt);

    return &btns->buf[index];
}

static inline bool LoadButtonCollider(s_rect* const collider, const s_button* const btn, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    assert(collider && IS_ZERO(*collider));
    assert(btn && IsButtonValid(btn));

    return LoadStrCollider(collider, btn->str, BUTTON_FONT, fonts, btn->pos, ek_str_hor_align_center, ek_str_ver_align_center, temp_mem_arena);
}

bool LoadIndexOfFirstButtonContainingPoint(int* const index, s_buttons* const btns, const s_vec_2d pt, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    assert(index);
    assert(btns && IsButtonsValid(btns));

    *index = -1;

    for (int i = 0; i < btns->cnt; i++) {
        const s_button* const btn = GetButton(btns, i);

        s_rect btn_str_collider = {0};

        if (!LoadButtonCollider(&btn_str_collider, btn, fonts, temp_mem_arena)) {
            return false;
        }

        if (IsPointInRect(pt, btn_str_collider)) {
            *index = i;
            break;
        }
    }

    return true;
}

bool RenderButton(s_rendering_context* const rendering_context, const s_button* const btn, const bool hovered, const s_fonts* const fonts, s_mem_arena* const temp_mem_arena) {
    return RenderStr(rendering_context, btn->str, BUTTON_FONT, fonts, btn->pos, ek_str_hor_align_center, ek_str_ver_align_center, hovered ? BUTTON_COLOR_HOVER : BUTTON_COLOR, temp_mem_arena);
}

#pragma once

#include QMK_KEYBOARD_H
#include "keymap_finnish.h"

/* Move the shared enums out of keymap.c so display.c and pointing.c can see them. */
enum charybdis_keymap_layers {
    LAYER_BASE = 0,
    LAYER_NUMBERS,
    LAYER_NAVIGATION,
    LAYER_FUNCTION,
    LAYER_MEDIA,
    LAYER_POINTER,
    LAYER_MACRO,
};

enum custom_keycodes {
    SET_MS_L = SAFE_RANGE,
    SET_MS_R,
};

#include "layout_positions.h"

/* Shared by pointing.c */
/* Automatically enable sniping-mode on the pointer layer */
#define CHARYBDIS_AUTO_SNIPING_ON_LAYER LAYER_POINTER

/* Shared pointing state helpers used by keymap.c and pointing.c */
void fleetsing_set_scrolling_enabled(bool enabled);
bool fleetsing_get_scrolling_enabled(void);

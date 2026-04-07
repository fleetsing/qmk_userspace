#pragma once

#include QMK_KEYBOARD_H
#include "keymap_finnish.h"

/* Shared enums used across the fleetsing userspace. */
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

/* Shared pointing state helpers used by the userspace modules. */
void fleetsing_set_scrolling_enabled(bool enabled);
bool fleetsing_get_scrolling_enabled(void);
bool fleetsing_pointing_process_record(uint16_t keycode, keyrecord_t *record);
bool fleetsing_repeat_process_record(uint16_t keycode, keyrecord_t *record);
bool fleetsing_autoshift_haptic_process_record(uint16_t keycode, keyrecord_t *record);
void fleetsing_autoshift_haptic_matrix_scan(void);

/* Layout helpers can override this if a keyboard wants different behaviour. */
#ifndef FLEETSING_AUTO_SNIPING_LAYER
#    define FLEETSING_AUTO_SNIPING_LAYER LAYER_POINTER
#endif

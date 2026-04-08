#pragma once

#include QMK_KEYBOARD_H
#include "keymap_finnish.h"

/*
 * Shared layer ids.
 *
 * The numeric order matters because keymap.c indexes directly by these enum
 * values, and OLED / feature logic refers to the same ids by name.
 */
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
    /*
     * SAFE_RANGE keeps these custom keycodes clear of the keyboard's built-in
     * and feature-provided keycode ranges.
     *
     * These select which combined pointing-device report is converted into
     * scroll.
     */
    SET_MS_L = SAFE_RANGE,
    SET_MS_R,
};

typedef enum {
    /* This state is runtime-only; it is not persisted in EEPROM. */
    FLEETSING_SCROLL_SIDE_LEFT = 0,
    FLEETSING_SCROLL_SIDE_RIGHT,
} fleetsing_scroll_side_t;

/*
 * Shared state and hook helpers used across the userspace modules.
 *
 * Scroll-side selection is userspace-owned. Sensor DPI is intentionally not:
 * that remains in the Charybdis firmware layer so runtime DPI/sniping keycodes
 * and EEPROM-backed state stay coherent.
 */
void                    fleetsing_set_scroll_side(fleetsing_scroll_side_t side);
fleetsing_scroll_side_t fleetsing_get_scroll_side(void);
bool                    fleetsing_pointing_process_record(uint16_t keycode, keyrecord_t *record);
bool                    fleetsing_autoshift_haptic_process_record(uint16_t keycode, keyrecord_t *record);
void                    fleetsing_autoshift_haptic_matrix_scan(void);

/*
 * Layout helpers can override this if a keyboard wants a different "precision
 * pointer" layer. Keep this as a real layer id; layer_state_set_user() treats
 * it as a single layer to compare against, not a bitmask.
 */
#ifndef FLEETSING_AUTO_SNIPING_LAYER
#    define FLEETSING_AUTO_SNIPING_LAYER LAYER_POINTER
#endif

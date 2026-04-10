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
    LAYER_NUMWORD,
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
     * These control userspace-owned board behavior.
     */
    NUMWORD = SAFE_RANGE,
    NUMLOCK,
    SET_MS_L,
    SET_MS_R,
    OS_MAC,
    OS_PC,
};

typedef enum {
    /* This state is runtime-only; it is not persisted in EEPROM. */
    FLEETSING_SCROLL_SIDE_LEFT = 0,
    FLEETSING_SCROLL_SIDE_RIGHT,
} fleetsing_scroll_side_t;

typedef enum {
    FLEETSING_OS_MAC = 0,
    FLEETSING_OS_PC,
} fleetsing_os_mode_t;

typedef enum {
    FLEETSING_HAPTIC_SCROLL_SIDE = 0,
    FLEETSING_HAPTIC_POINTER_LAYER_ON,
    FLEETSING_HAPTIC_POINTER_LAYER_OFF,
    FLEETSING_HAPTIC_OS_MAC,
    FLEETSING_HAPTIC_OS_PC,
    FLEETSING_HAPTIC_LAYER_LOCK_ON,
    FLEETSING_HAPTIC_LAYER_LOCK_OFF,
    FLEETSING_HAPTIC_MACRO_RECORD_START,
    FLEETSING_HAPTIC_MACRO_RECORD_STOP,
    FLEETSING_HAPTIC_MACRO_PLAY,
    FLEETSING_HAPTIC_EEPROM_CLEAR,
    FLEETSING_HAPTIC_BOOTLOADER,
} fleetsing_haptic_event_t;

/*
 * Shared state and hook helpers used across the userspace modules.
 *
 * Scroll-side selection is userspace-owned. Sensor DPI is intentionally not:
 * that remains in the Charybdis firmware layer so runtime DPI/sniping keycodes
 * and EEPROM-backed state stay coherent.
 */
void                    fleetsing_set_scroll_side(fleetsing_scroll_side_t side);
fleetsing_scroll_side_t fleetsing_get_scroll_side(void);
fleetsing_os_mode_t     fleetsing_get_os_mode(void);
void                    fleetsing_set_os_mode(fleetsing_os_mode_t mode);
const char             *fleetsing_get_os_mode_name(void);
uint16_t                fleetsing_os_keycode(uint16_t mac_keycode, uint16_t pc_keycode);
void                    fleetsing_haptic_play_event(fleetsing_haptic_event_t event);
/*
 * Key and pointing modules call this to reset the OLED idle timer after real
 * user activity. The timer itself stays private to the display module.
 */
void fleetsing_display_note_activity(void);
bool fleetsing_os_process_record(uint16_t keycode, keyrecord_t *record);
bool fleetsing_pointing_process_record(uint16_t keycode, keyrecord_t *record);
bool fleetsing_numword_process_record(uint16_t keycode, keyrecord_t *record);
bool fleetsing_numword_is_active(void);
uint16_t fleetsing_numword_idle_remaining(void);
bool fleetsing_numword_display_is_active(void);
uint16_t fleetsing_numword_display_remaining(void);
void fleetsing_display_post_init(void);
void fleetsing_display_sync_task(void);
bool fleetsing_autoshift_haptic_process_record(uint16_t keycode, keyrecord_t *record);
void fleetsing_numword_task(void);
void fleetsing_autoshift_haptic_matrix_scan(void);

/*
 * Layout helpers can override this if a keyboard wants a different "precision
 * pointer" layer. Keep this as a real layer id; layer_state_set_user() treats
 * it as a single layer to compare against, not a bitmask.
 */
#ifndef FLEETSING_AUTO_SNIPING_LAYER
#    define FLEETSING_AUTO_SNIPING_LAYER LAYER_POINTER
#endif

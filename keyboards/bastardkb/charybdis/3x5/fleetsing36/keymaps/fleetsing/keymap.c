/*
 * My personal keymap for the Charybdis Nano keyboard.
 * Uses a custom keyboard layout designed for writing English and Finnish.
 * Assumes the OS language is set to Finnish.
 */

#include QMK_KEYBOARD_H
#include "keymap_finnish.h"
#include "fleetsing.h"
/* Positional aliases keep the matrix readable while preserving per-key behaviors. */
#include "layouts/charybdis_3x5/layout_positions.h"
/* Included here so QMK keymap introspection can still see key_combos. */
#include "layouts/charybdis_3x5/combos.def"

/*
 * Layer-local shorthand aliases.
 *
 * QMK names such as LSFT_T(), LT(), and OSM() are compact once familiar, but
 * they make the matrix hard to scan in bulk. These aliases keep the layer
 * definitions focused on layout intent instead of modifier boilerplate.
 */
// Mod-tap shortcuts for left-hand function layer.
#define _SFT_F17 LSFT_T(KC_F17)
#define _OPT_F18 RALT_T(KC_F18)
#define _CTL_F19 LCTL_T(KC_F19)
#define _GUI_F20 LGUI_T(KC_F20)

// Mod-tap shortcuts for right-hand function layer.
#define _GUI_F4 RGUI_T(KC_F4)
#define _CTL_F5 RCTL_T(KC_F5)
#define _OPT_F6 RALT_T(KC_F6)
#define _SFT_F11 RSFT_T(KC_F11)

// Mod-tap shortcuts for numbers layer.
#define _CTL_LEFT RCTL_T(KC_LEFT)
#define _GUI_RIGHT RGUI_T(KC_RIGHT)

// Mod-tap shortcuts for navigation layer.
#define _MEH_LEFT MEH_T(KC_LEFT)
#define _GUI_DOWN RGUI_T(KC_DOWN)
#define _CTL_UP RCTL_T(KC_UP)
#define _OPT_RIGHT RALT_T(KC_RIGHT)

#ifndef POINTING_DEVICE_ENABLE
/*
 * Keep the keymap buildable without the pointing feature enabled. These fall
 * back to inert keycodes so the layer matrix can still compile cleanly.
 */
#    define DPI_MOD KC_NO
#    define S_D_MOD KC_NO
#    define S_D_RMOD KC_NO
#    define SNIPING KC_NO
#endif // !POINTING_DEVICE_ENABLE

// clang-format off

/**
 * \brief Base typing layer.
 *
 * The actual alpha bindings mostly live in layout_positions.h so changes to a
 * physical position can be made once and then reused across combos and layers.
 */
#define LAYOUT_LAYER_BASE                                                                                                           \
    _L15,       _L14,       _L13,       _L12,       _L11,               _R11,       _R12,       _R13,       _R14,       _R15,       \
    _L25,       _L24,       _L23,       _L22,       _L21,               _R21,       _R22,       _R23,       _R24,       _R25,       \
    _L35,       _L34,       _L33,       _L32,       _L31,               _R31,       _R32,       _R33,       _R34,       _R35,       \
                            _L43,       _L42,       _L41,               _R41,       _R42,       _R43

/**
 * \brief Number-entry layer.
 *
 * The left hand holds editing helpers and one-shot modifiers while the right
 * hand carries the number cluster. QK_LLCK keeps the layer latched until it is
 * pressed again or QMK's layer-lock timeout expires.
 */
#define LAYOUT_LAYER_NUMBERS                                                                                                        \
    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,            XXXXXXX,    FI_7,       FI_8,       FI_9,       XXXXXXX,    \
    OSM(MOD_LSFT), OSM(MOD_RALT), _CTL_LEFT, _GUI_RIGHT, KC_DEL,        KC_BSPC,    FI_4,       FI_5,       FI_6,       FI_0,       \
    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,            XXXXXXX,    FI_1,       FI_2,       FI_3,       XXXXXXX,    \
                            KC_SPC,     XXXXXXX,    _______,            QK_LLCK,    KC_ENT,     XXXXXXX

/** 
* \brief Navigation layer.
*
* Intended for cursor movement and document navigation without leaving the home
* block. The rightmost one-shot Shift lets selections extend without holding a
* modifier continuously.
*/
#define LAYOUT_LAYER_NAVIGATION                                                                                                     \
    XXXXXXX,    S(KC_TAB),  KC_UP,      KC_TAB,     KC_PGUP,            KC_PGUP,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    \
    KC_LSFT,    KC_LEFT,    KC_DOWN,    KC_RGHT,    KC_DEL,             _MEH_LEFT,  _GUI_DOWN,  _CTL_UP,    _OPT_RIGHT, OSM(MOD_LSFT), \
    XXXXXXX,    KC_END,     KC_INS,     KC_HOME,    KC_PGDN,            KC_PGDN,    KC_HOME,    KC_INS,     KC_END,     XXXXXXX,    \
                            _______,    XXXXXXX,    KC_ENT,             QK_LLCK,    KC_ENT,     XXXXXXX

/**
 * \brief Function-key layer. */
#define LAYOUT_LAYER_FUNCTION                                                                                                       \
    KC_F21,     KC_F22,     KC_F23,     KC_F24,     KC_PSCR,            KC_PSCR,    KC_F7,      KC_F8,      KC_F9,      KC_F12,     \
    _SFT_F17,   _OPT_F18,   _CTL_F19,   _GUI_F20,   KC_SCRL,            KC_SCRL,    _GUI_F4,    _CTL_F5,    _OPT_F6,    _SFT_F11,   \
    KC_F13,     KC_F14,     KC_F15,     KC_F16,     KC_PAUS,            KC_PAUS,    KC_F1,      KC_F2,      KC_F3,      KC_F10,     \
                            XXXXXXX,    XXXXXXX,    XXXXXXX,            _______,    XXXXXXX,    XXXXXXX

/**
 * \brief Media/system layer.
 *
 * EE_CLR clears persisted QMK settings from EEPROM. QK_BOOT jumps into the
 * bootloader for flashing.
 */
#define LAYOUT_LAYER_MEDIA                                                                                                          \
    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,            XXXXXXX,    XXXXXXX,    KC_UP,      XXXXXXX,    XXXXXXX,    \
    KC_MPRV,    KC_VOLD,    KC_MUTE,    KC_VOLU,    KC_MNXT,            XXXXXXX,    KC_LEFT,    KC_DOWN,    KC_RGHT,    XXXXXXX,    \
    XXXXXXX,    XXXXXXX,    XXXXXXX,    EE_CLR,     QK_BOOT,            XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    \
                            KC_MPLY,    XXXXXXX,    KC_MSTP,            XXXXXXX,    XXXXXXX,    XXXXXXX

/**
 * \brief Pointer and mouse-button layer.
 *
 * This layer does not choose the active sensor DPI directly. It toggles
 * Charybdis sniping behavior and exposes sniping-DPI controls while userspace
 * decides which sensor's motion is converted into scroll.
 */
#define LAYOUT_LAYER_POINTER                                                                                                        \
    /* Pointer-layer DPI controls are sniping-DPI controls because this layer auto-enables sniping mode. */                       \
    QK_BOOT,    EE_CLR,     XXXXXXX,    S_D_RMOD,   S_D_MOD,            S_D_MOD,    S_D_RMOD,   XXXXXXX,    EE_CLR,     QK_BOOT,    \
    KC_LSFT,    KC_RALT,    KC_LCTL,    KC_LGUI,    XXXXXXX,            XXXXXXX,    KC_RGUI,    KC_RCTL,    KC_RALT,    KC_RSFT,    \
    _______,    DRGSCRL,    XXXXXXX,    SET_MS_L,   XXXXXXX,            XXXXXXX,    SET_MS_R,   XXXXXXX,    DRGSCRL,    _______,    \
                            MS_BTN2,    MS_BTN1,    MS_BTN3,            MS_BTN3,    MS_BTN1,    MS_BTN2

/**
 * \brief Dynamic-macro and desktop-shortcut layer. */
#define LAYOUT_LAYER_MACRO                                                                                                          \
    XXXXXXX,    XXXXXXX,    DM_REC2,    DM_REC1,    XXXXXXX,            XXXXXXX,    XXXXXXX,    KC_UP,      XXXXXXX,    XXXXXXX,    \
    XXXXXXX,    XXXXXXX,    DM_PLY2,    DM_PLY1,    DM_RSTP,            XXXXXXX,    KC_LEFT,    KC_DOWN,    KC_RGHT,    XXXXXXX,    \
    XXXXXXX,    G(FI_Z),    G(FI_X),    G(FI_C),    G(FI_V),            XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    \
                            XXXXXXX,    XXXXXXX,    XXXXXXX,            XXXXXXX,    XXXXXXX,    XXXXXXX

/* Wrap the board's LAYOUT macro so layer macros stay visually grouped. */
#define LAYOUT_wrapper(...) LAYOUT(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_BASE]       = LAYOUT_wrapper(LAYOUT_LAYER_BASE),
    [LAYER_NUMBERS]    = LAYOUT_wrapper(LAYOUT_LAYER_NUMBERS),
    [LAYER_NAVIGATION] = LAYOUT_wrapper(LAYOUT_LAYER_NAVIGATION),
    [LAYER_FUNCTION]   = LAYOUT_wrapper(LAYOUT_LAYER_FUNCTION),
    [LAYER_MEDIA]      = LAYOUT_wrapper(LAYOUT_LAYER_MEDIA),
    [LAYER_POINTER]    = LAYOUT_wrapper(LAYOUT_LAYER_POINTER),
    [LAYER_MACRO]      = LAYOUT_wrapper(LAYOUT_LAYER_MACRO),
};
// clang-format on

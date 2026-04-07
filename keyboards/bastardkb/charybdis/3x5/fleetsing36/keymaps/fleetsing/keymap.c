/*
 * My personal keymap for the Charybdis Nano keyboard.
 * Uses a custom keyboard layout designed for writing English and Finnish.
 * Assumes the OS language is set to Finnish.
 */

#include QMK_KEYBOARD_H
#include "keymap_finnish.h"
#include "fleetsing.h"

// Tap Dance declarations
enum {
    TD_SHIFT,
};

// Tap Dance definitions
tap_dance_action_t tap_dance_actions[] = {
    // Tap once for Escape, twice for Caps Lock
    [TD_SHIFT] = ACTION_TAP_DANCE_DOUBLE(OSM(MOD_LSFT), CW_TOGG),
};

// Left-hand row 1.
#define _L15 FI_Q
#define _L14 FI_W
#define _L13 FI_E
#define _L12 FI_R
#define _L11 FI_T

// Left-hand row 2.
#define _L25 LSFT_T(FI_A)
#define _L24 RALT_T(FI_S)
#define _L23 LCTL_T(FI_D)
#define _L22 LGUI_T(FI_F)
#define _L21 FI_G

// Left-hand row 3.
#define _L35 LT(LAYER_POINTER, FI_Z)
#define _L34 FI_X
#define _L33 MEH_T(FI_C)
#define _L32 HYPR_T(FI_V)
#define _L31 FI_B

// Left-hand thumb cluster.
#define _L43 LT(LAYER_NUMBERS, KC_SPC)
#define _L42 MS_BTN1
#define _L41 LT(LAYER_FUNCTION, KC_ESC)

// Right-hand row 1.
#define _R11 FI_Y
#define _R12 FI_U
#define _R13 FI_I
#define _R14 FI_O
#define _R15 FI_P

// Right-hand row 2.
#define _R21 FI_H
#define _R22 RGUI_T(FI_J)
#define _R23 RCTL_T(FI_K)
#define _R24 RALT_T(FI_L)
#define _R25 RSFT_T(FI_ODIA)

// Right-hand row 3.
#define _R31 FI_N
#define _R32 HYPR_T(FI_M)
#define _R33 MEH_T(FI_COMM)
#define _R34 FI_DOT
#define _R35 LT(LAYER_POINTER, FI_ADIA)

// Right-hand thumb cluster.
#define _R41 LT(LAYER_MEDIA, KC_ESC)
#define _R42 MS_BTN1
#define _R43 LT(LAYER_NAVIGATION, KC_ENT)

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
# define _GUI_RIGHT RGUI_T(KC_RIGHT)

// Mod-tap shortcuts for navigation layer.
#define _MEH_LEFT MEH_T(KC_LEFT)
#define _GUI_DOWN RGUI_T(KC_DOWN)
#define _CTL_UP RCTL_T(KC_UP)
#define _OPT_RIGHT RALT_T(KC_RIGHT)

#ifndef POINTING_DEVICE_ENABLE
// #    define DRGSCRL KC_NO
#    define DPI_MOD KC_NO
#    define S_D_MOD KC_NO
#    define SNIPING KC_NO
#endif // !POINTING_DEVICE_ENABLE

enum combos {
    L24_L23,
    L34_L33,
    L14_L13,
    L23_L22,
    L42_L41,
    L43_L41,
    L43_L42,

    R23_R24,
    R34_R33,
    R13_R14,
    R22_R23,
    R41_R42,
    R41_R43,
    R42_R43,

    L11_L21,
    L21_L31,
    L12_L22,
    L22_L32,
    L13_L23,
    L23_L33,
    L14_L24,
    L24_L34,

    R11_R21,
    R21_R31,
    R12_R22,
    R22_R32,
    R13_R23,
    R23_R33,
    R14_R24,
    R24_R34,
};

// Left-hand horizontal combos.
const uint16_t PROGMEM L24_L23_COMBO[] = {_L24, _L23, COMBO_END};
const uint16_t PROGMEM L34_L33_COMBO[] = {_L34, _L33, COMBO_END};
const uint16_t PROGMEM L14_L13_COMBO[] = {_L14, _L13, COMBO_END};
const uint16_t PROGMEM L23_L22_COMBO[] = {_L23, _L22, COMBO_END};
const uint16_t PROGMEM L42_L41_COMBO[] = {_L42, _L41, COMBO_END};
const uint16_t PROGMEM L43_L41_COMBO[] = {_L43, _L41, COMBO_END};
const uint16_t PROGMEM L43_L42_COMBO[] = {_L43, _L42, COMBO_END};

// Right-hand horizontal combos.
const uint16_t PROGMEM R23_R24_COMBO[] = {_R23, _R24, COMBO_END};
const uint16_t PROGMEM R34_R33_COMBO[] = {_R33, _R34, COMBO_END};
const uint16_t PROGMEM R13_R14_COMBO[] = {_R13, _R14, COMBO_END};
const uint16_t PROGMEM R22_R23_COMBO[] = {_R22, _R23, COMBO_END};
const uint16_t PROGMEM R41_R42_COMBO[] = {_R41, _R42, COMBO_END};
const uint16_t PROGMEM R41_R43_COMBO[] = {_R41, _R43, COMBO_END};
const uint16_t PROGMEM R42_R43_COMBO[] = {_R42, _R43, COMBO_END};

// Left-hand vertical combos.
const uint16_t PROGMEM L11_L21_COMBO[] = {_L11, _L21, COMBO_END};
const uint16_t PROGMEM L21_L31_COMBO[] = {_L21, _L31, COMBO_END};
const uint16_t PROGMEM L12_L22_COMBO[] = {_L12, _L22, COMBO_END};
const uint16_t PROGMEM L22_L32_COMBO[] = {_L22, _L32, COMBO_END};
const uint16_t PROGMEM L13_L23_COMBO[] = {_L13, _L23, COMBO_END};
const uint16_t PROGMEM L23_L33_COMBO[] = {_L23, _L33, COMBO_END};
const uint16_t PROGMEM L14_L24_COMBO[] = {_L14, _L24, COMBO_END};
const uint16_t PROGMEM L24_L34_COMBO[] = {_L24, _L34, COMBO_END};

// Right-hand vertical combos.
const uint16_t PROGMEM R11_R21_COMBO[] = {_R11, _R21, COMBO_END};
const uint16_t PROGMEM R21_R31_COMBO[] = {_R21, _R31, COMBO_END};
const uint16_t PROGMEM R12_R22_COMBO[] = {_R12, _R22, COMBO_END};
const uint16_t PROGMEM R22_R32_COMBO[] = {_R22, _R32, COMBO_END};
const uint16_t PROGMEM R13_R23_COMBO[] = {_R13, _R23, COMBO_END};
const uint16_t PROGMEM R23_R33_COMBO[] = {_R23, _R33, COMBO_END};
const uint16_t PROGMEM R14_R24_COMBO[] = {_R14, _R24, COMBO_END};
const uint16_t PROGMEM R24_R34_COMBO[] = {_R24, _R34, COMBO_END};

combo_t key_combos[] = {
    // Left-hand horizontal combos.
    [L24_L23] = COMBO(L24_L23_COMBO, FI_COMM),
    [L34_L33] = COMBO(L34_L33_COMBO, FI_COLN),
    [L14_L13] = COMBO(L14_L13_COMBO, FI_PLUS),
    [L23_L22] = COMBO(L23_L22_COMBO, S(KC_TAB)),
    [L42_L41] = COMBO(L42_L41_COMBO, MS_BTN3),
    [L43_L41] = COMBO(L43_L41_COMBO, QK_REP),
    [L43_L42] = COMBO(L43_L42_COMBO, MS_BTN2),

    // Right-hand horizontal combos.
    [R23_R24] = COMBO(R23_R24_COMBO, FI_DOT),
    [R34_R33] = COMBO(R34_R33_COMBO, FI_QUOT),
    [R13_R14] = COMBO(R13_R14_COMBO, FI_MINS),
    [R22_R23] = COMBO(R22_R23_COMBO, KC_TAB),
    [R41_R42] = COMBO(R41_R42_COMBO, MS_BTN3),
    [R41_R43] = COMBO(R41_R43_COMBO, QK_AREP),
    [R42_R43] = COMBO(R42_R43_COMBO, MS_BTN2),

    // Left-hand vertical combos.
    [L11_L21] = COMBO(L11_L21_COMBO, S(FI_4)),
    [L21_L31] = COMBO(L21_L31_COMBO, A(FI_2)),
    [L12_L22] = COMBO(L12_L22_COMBO, FI_DOT),
    [L22_L32] = COMBO(L22_L32_COMBO, FI_COMM),
    [L13_L23] = COMBO(L13_L23_COMBO, FI_PLUS),
    [L23_L33] = COMBO(L23_L33_COMBO, FI_MINS),
    [L14_L24] = COMBO(L14_L24_COMBO, S(FI_7)),
    [L24_L34] = COMBO(L24_L34_COMBO, S(FI_QUOT)),

    // Right-hand vertical combos.
    [R11_R21] = COMBO(R11_R21_COMBO, S(FI_DOT)),
    [R21_R31] = COMBO(R21_R31_COMBO, A(FI_DIAE)),
    [R12_R22] = COMBO(R12_R22_COMBO, S(FI_8)),
    [R22_R32] = COMBO(R22_R32_COMBO, A(FI_8)),
    [R13_R23] = COMBO(R13_R23_COMBO, S(FI_9)),
    [R23_R33] = COMBO(R23_R33_COMBO, A(FI_9)),
    [R14_R24] = COMBO(R14_R24_COMBO, FI_QUOT),
    [R24_R34] = COMBO(R24_R34_COMBO, S(FI_6)),
};

// clang-format off

/**
 * \brief QWERTY layout (3 rows, 10 columns).*/
#define LAYOUT_LAYER_BASE                                                                                                           \
    _L15,       _L14,       _L13,       _L12,       _L11,               _R11,       _R12,       _R13,       _R14,       _R15,       \
    _L25,       _L24,       _L23,       _L22,       _L21,               _R21,       _R22,       _R23,       _R24,       _R25,       \
    _L35,       _L34,       _L33,       _L32,       _L31,               _R31,       _R32,       _R33,       _R34,       _R35,       \
                            _L43,       _L42,       _L41,               _R41,       _R42,       _R43

/**
 * \brief NUMBERS layout. */
#define LAYOUT_LAYER_NUMBERS                                                                                                        \
    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,            XXXXXXX,    FI_7,       FI_8,       FI_9,       XXXXXXX,    \
    MOD_LSFT,   MOD_RALT,   _CTL_LEFT,  _GUI_RIGHT, KC_DEL,             KC_BSPC,    FI_4,       FI_5,       FI_6,       FI_0,       \
    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,            XXXXXXX,    FI_1,       FI_2,       FI_3,       XXXXXXX,    \
                            KC_SPC,     XXXXXXX,    _______,            QK_LLCK,    KC_ENT,     XXXXXXX

/** 
* \brief Navigation layer. */
#define LAYOUT_LAYER_NAVIGATION                                                                                                     \
    XXXXXXX,    S(KC_TAB),  KC_UP,      KC_TAB,     KC_PGUP,            KC_PGUP,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    \
    KC_LSFT,    KC_LEFT,    KC_DOWN,    KC_RGHT,    KC_DEL,             _MEH_LEFT,  _GUI_DOWN,  _CTL_UP,    _OPT_RIGHT, MOD_LSFT,   \
    XXXXXXX,    KC_END,     KC_INS,     KC_HOME,    KC_PGDN,            KC_PGDN,    KC_HOME,    KC_INS,     KC_END,     XXXXXXX,    \
                            _______,    XXXXXXX,    KC_ENT,             QK_LLCK,    KC_ENT,     XXXXXXX

/**
 * \brief Function layer. */
#define LAYOUT_LAYER_FUNCTION                                                                                                       \
    KC_F21,     KC_F22,     KC_F23,     KC_F24,     KC_PSCR,            KC_PSCR,    KC_F7,      KC_F8,      KC_F9,      KC_F12,     \
    _SFT_F17,   _OPT_F18,   _CTL_F19,   _GUI_F20,   KC_SCRL,            KC_SCRL,    _GUI_F4,    _CTL_F5,    _OPT_F6,    _SFT_F11,   \
    KC_F13,     KC_F14,     KC_F15,     KC_F16,     KC_PAUS,            KC_PAUS,    KC_F1,      KC_F2,      KC_F3,      KC_F10,     \
                            XXXXXXX,    XXXXXXX,    XXXXXXX,            _______,    XXXXXXX,    XXXXXXX

/**
 * \brief Media layer. */
#define LAYOUT_LAYER_MEDIA                                                                                                          \
    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,            XXXXXXX,    XXXXXXX,    KC_UP,      XXXXXXX,    XXXXXXX,    \
    KC_MPRV,    KC_VOLD,    KC_MUTE,    KC_VOLU,    KC_MNXT,            XXXXXXX,    KC_LEFT,    KC_DOWN,    KC_RGHT,    XXXXXXX,    \
    XXXXXXX,    XXXXXXX,    XXXXXXX,    EE_CLR,     QK_BOOT,            XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    \
                            KC_MPLY,    XXXXXXX,    KC_MSTP,            XXXXXXX,    XXXXXXX,    XXXXXXX

/**
 * \brief Mouse emulation and pointer functions. */
#define LAYOUT_LAYER_POINTER                                                                                                        \
    QK_BOOT,    EE_CLR,     XXXXXXX,    DPI_MOD,    S_D_MOD,            S_D_MOD,    DPI_MOD,    XXXXXXX,    EE_CLR,     QK_BOOT,    \
    KC_LSFT,    KC_RALT,    KC_LCTL,    KC_LGUI,    XXXXXXX,            XXXXXXX,    KC_RGUI,    KC_RCTL,    KC_RALT,    KC_RSFT,    \
    _______,    DRGSCRL,    XXXXXXX,    SET_MS_L,   XXXXXXX,            XXXXXXX,    SET_MS_R,   XXXXXXX,    DRGSCRL,    _______,    \
                            MS_BTN2,    MS_BTN1,    MS_BTN3,            MS_BTN3,    MS_BTN1,    MS_BTN2

/**
 * \brief Macro layer. */
#define LAYOUT_LAYER_MACRO                                                                                                          \
    XXXXXXX,    XXXXXXX,    DM_REC2,    DM_REC1,    XXXXXXX,            XXXXXXX,    XXXXXXX,    KC_UP,      XXXXXXX,    XXXXXXX,    \
    XXXXXXX,    XXXXXXX,    DM_PLY2,    DM_PLY1,    DM_RSTP,            XXXXXXX,    KC_LEFT,    KC_DOWN,    KC_RGHT,    XXXXXXX,    \
    XXXXXXX,    G(FI_Z),    G(FI_X),    G(FI_C),    G(FI_V),            XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    \
                            XXXXXXX,    XXXXXXX,    XXXXXXX,            XXXXXXX,    XXXXXXX,    XXXXXXX

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

#ifdef POINTING_DEVICE_ENABLE
#    ifdef CHARYBDIS_AUTO_SNIPING_ON_LAYER
layer_state_t layer_state_set_user(layer_state_t state) {
    charybdis_set_pointer_sniping_enabled(layer_state_cmp(state, CHARYBDIS_AUTO_SNIPING_ON_LAYER));
    return state;
}
#    endif // CHARYBDIS_AUTO_SNIPING_ON_LAYER
#endif     // POINTING_DEVICE_ENABLE

#ifdef RGB_MATRIX_ENABLE
// Forward-declare this helper function since it is defined in
// rgb_matrix.c.
void rgb_matrix_update_pwm_buffers(void);
#endif

bool remember_last_key_user(uint16_t keycode, keyrecord_t* record, uint8_t* remembered_mods) {
    if (keycode == QK_REP) { return false; }
    if (keycode == QK_AREP) { return false; }
    return true;
}

// bool get_auto_shifted_key(uint16_t keycode, keyrecord_t *record) {
//     return get_custom_auto_shifted_key(keycode, record);
// }

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case SET_MS_L:
            if (record->event.pressed) {
                fleetsing_set_scrolling_enabled(true);
            }
            return false; // Skip all further processing of this key
        case SET_MS_R:
            if (record->event.pressed) {
                fleetsing_set_scrolling_enabled(false);
            }
            return false; // Skip all further processing of this key
        case QK_AREP:
            if (record->tap.count) {  // On tap.
                alt_repeat_key_invoke(&record->event);  // Alt repeat the last key.
                return false;  // Skip default handling.
            }
            return true;
        case QK_REP:
            if (record->tap.count) {  // On tap.
                repeat_key_invoke(&record->event);  // Repeat the last key.
                return false;  // Skip default handling.
            }
            return true;
        default:
            return true; // Process all other keycodes normally
    }
}

bool get_custom_auto_shifted_key(uint16_t keycode, keyrecord_t *record) {
    switch(keycode) {
        case S(FI_4):
            return true;
        case A(FI_2):
            return true;
        case FI_DOT:
            return true;
        case FI_COMM:
            return true;
        case FI_PLUS:
            return true;
        case FI_MINS:
            return true;
        case S(FI_7):
            return true;
        case S(FI_QUOT):
            return true;
        case S(FI_DOT):
            return true;
        case A(FI_DIAE):
            return true;
        case S(FI_8):
            return true;
        case A(FI_8):
            return true;
        case S(FI_9):
            return true;
        case A(FI_9):
            return true;
        case FI_QUOT:
            return true;
        case S(FI_6):
            return true;
        case _L25:
            return true;
        case _L24:
            return true;
        case _L23:
            return true;
        case _L22:
            return true;
        case _L32:
            return true;
        case _L33:
            return true;
        case _R35:
            return true;
        case _R33:
            return true;
        case _R32:
            return true;
        case _R22:
            return true;
        case _R23:
            return true;
        case _R24:
            return true;
        case _R25:
            return true;
        default:
            return false;
    }
}

void autoshift_press_user(uint16_t keycode, bool shifted, keyrecord_t *record) {
    switch(keycode) {
        case S(FI_4):
            register_code16((!shifted) ? S(FI_4) : A(FI_4));
            break;
        case A(FI_2):
            register_code16((!shifted) ? A(FI_2) : S(FI_DIAE));
            break;
        case FI_DOT:
            register_code16((!shifted) ? FI_DOT : S(FI_PLUS));
            break;
        case FI_COMM:
            register_code16((!shifted) ? FI_COMM : S(FI_1));
            break;
        case FI_PLUS:
            register_code16((!shifted) ? FI_PLUS : S(FI_0));
            break;
        case FI_MINS:
            register_code16((!shifted) ? FI_MINS : S(FI_MINS));
            break;
        case S(FI_7):
            register_code16((!shifted) ? S(FI_7) : S(A(FI_7)));
            break;
        case S(FI_QUOT):
            register_code16((!shifted) ? S(FI_QUOT) : S(FI_3));
            break;
        case S(FI_DOT):
            register_code16((!shifted) ? S(FI_DOT) : S(FI_COMM));
            break;
        case A(FI_DIAE):
            register_code16((!shifted) ? A(FI_DIAE) : A(FI_7));
            break;
        case S(FI_8):
            register_code16((!shifted) ? S(FI_8) : S(A(FI_8)));
            break;
        case A(FI_8):
            register_code16((!shifted) ? A(FI_8) : FI_SECT);
            break;
        case S(FI_9):
            register_code16((!shifted) ? S(FI_9) : S(A(FI_9)));
            break;
        case A(FI_9):
            register_code16((!shifted) ? A(FI_9) : S(FI_SECT));
            break;
        case FI_QUOT:
            register_code16((!shifted) ? FI_QUOT : S(FI_2));
            break;
        case S(FI_6):
            register_code16((!shifted) ? S(FI_6) : S(FI_5));
            break;
        default:
            if (shifted) {
                add_weak_mods(MOD_BIT(KC_LSFT));
            }
            // & 0xFF gets the Tap key for Tap Holds, required when using Retro Shift
            register_code16((IS_RETRO(keycode)) ? keycode & 0xFF : keycode);
    }
}

void autoshift_release_user(uint16_t keycode, bool shifted, keyrecord_t *record) {
    switch(keycode) {
        case S(FI_4):
            unregister_code16((!shifted) ? S(FI_4) : A(FI_4));
            if (shifted) { set_last_keycode(A(FI_4)); }
            break;
        case A(FI_2):
            unregister_code16((!shifted) ? A(FI_2) : S(FI_DIAE));
            if (shifted) { set_last_keycode(S(FI_DIAE)); }
            break;
        case FI_DOT:
            unregister_code16((!shifted) ? FI_DOT : S(FI_PLUS));
            if (shifted) { set_last_keycode(S(FI_PLUS)); }
            break;
        case FI_COMM:
            unregister_code16((!shifted) ? FI_COMM : S(FI_1));
            if (shifted) { set_last_keycode(S(FI_1)); }
            break;
        case FI_PLUS:
            unregister_code16((!shifted) ? FI_PLUS : S(FI_0));
            if (shifted) { set_last_keycode(S(FI_0)); }
            break;
        case FI_MINS:
            unregister_code16((!shifted) ? FI_MINS : S(FI_MINS));
            if (shifted) { set_last_keycode(S(FI_MINS)); }
            break;
        case S(FI_7):
            unregister_code16((!shifted) ? S(FI_7) : S(A(FI_7)));
            if (shifted) { set_last_keycode(S(A(FI_7))); }
            break;
        case S(FI_QUOT):
            unregister_code16((!shifted) ? S(FI_QUOT) : S(FI_3));
            if (shifted) { set_last_keycode(S(FI_3)); }
            break;
        case S(FI_DOT):
            unregister_code16((!shifted) ? S(FI_DOT) : S(FI_COMM));
            if (shifted) { set_last_keycode(S(FI_COMM)); }
            break;
        case A(FI_DIAE):
            unregister_code16((!shifted) ? A(FI_DIAE) : A(FI_7));
            if (shifted) { set_last_keycode(A(FI_7)); }
            break;
        case S(FI_8):
            unregister_code16((!shifted) ? S(FI_8) : S(A(FI_8)));
            if (shifted) { set_last_keycode(S(A(FI_8))); }
            break;
        case A(FI_8):
            unregister_code16((!shifted) ? A(FI_8) : FI_SECT);
            if (shifted) { set_last_keycode(FI_SECT); }
            break;
        case S(FI_9):
            unregister_code16((!shifted) ? S(FI_9) : S(A(FI_9)));
            if (shifted) { set_last_keycode(S(A(FI_9))); }
            break;
        case A(FI_9):
            unregister_code16((!shifted) ? A(FI_9) : S(FI_SECT));
            if (shifted) { set_last_keycode(S(FI_SECT)); }
            break;
        case FI_QUOT:
            unregister_code16((!shifted) ? FI_QUOT : S(FI_2));
            if (shifted) { set_last_keycode(S(FI_2)); }
            break;
        case S(FI_6):
            unregister_code16((!shifted) ? S(FI_6) : S(FI_5));
            if (shifted) { set_last_keycode(S(FI_5)); }
            break;
        default:
            // & 0xFF gets the Tap key for Tap Holds, required when using Retro Shift
            // The IS_RETRO check isn't really necessary here, always using
            // keycode & 0xFF would be fine.
            unregister_code16((IS_RETRO(keycode)) ? keycode & 0xFF : keycode);
    }
    // Remember that the key was autoshifted.
}

uint16_t get_alt_repeat_key_keycode_user(uint16_t keycode, uint8_t mods) {
    switch (keycode) {
        case KC_TAB: return S(KC_TAB);  // Tab reverses to Shift + Tab.
        case S(KC_TAB): return KC_TAB;  // Shift + Tab reverses to Tab.
        
        case G(FI_Y): return G(FI_Z);  // GUI + Y reverses to GUI + Z.
        case G(FI_Z): return G(FI_Y);  // GUI + Z reverses to GUI + Y.
        case G(FI_C): return G(FI_V);  // GUI + C reverses to GUI + V.

        case C(FI_Y): return C(FI_Z);  // Ctrl + Y reverses to Ctrl + Z.
        case C(FI_Z): return C(FI_Y);  // Ctrl + Z reverses to Ctrl + Y.
        case C(FI_C): return C(FI_V);  // Ctrl + C reverses to Ctrl + V.
    
        case FI_LPRN: return FI_RPRN;  // Left Parenthesis reverses to Right Parenthesis.
        case A(FI_8): return A(FI_9);  // Left Brakcet reverses to Right Bracket.
        case A(FI_LPRN): return A(FI_RPRN);  // Left Brace reverses to Right Brace.

        case FI_SECT: return FI_HALF;  // Less Than reverses to Greater Than.
        case FI_HALF: return FI_SECT;  // Greater Than reverses to Less Than.
    }
    return KC_TRNS;
}
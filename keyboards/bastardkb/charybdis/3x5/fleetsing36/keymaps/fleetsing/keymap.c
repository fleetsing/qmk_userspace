/*
 * Personal Charybdis 3x5 keymap for English and Finnish.
 *
 * The matrix assumes the host OS keyboard layout is Finnish and keeps the
 * physical-position aliases in layout_positions.h as the source of truth for
 * thumb roles and other dual-role base bindings.
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

/*
 * Left-hand navigation-layer one-shot mod aliases.
 *
 * Keep these in the same left-to-right order as the layer row they occupy.
 */
#define _OSM_LSFT OSM(MOD_LSFT)
#define _OSM_LALT OSM(MOD_LALT)
#define _OSM_LCTL OSM(MOD_LCTL)
#define _OSM_LGUI OSM(MOD_LGUI)
#define _OSM_MEH OSM(MOD_MEH)
#define _OSM_HYPR OSM(MOD_HYPR)

/*
 * Left-hand function-layer mod-tap aliases.
 *
 * Keep these in the same left-to-right order as the layer row they occupy.
 */
#define _SFT_F17 LSFT_T(KC_F17)
#define _OPT_F18 RALT_T(KC_F18)
#define _CTL_F19 LCTL_T(KC_F19)
#define _GUI_F20 LGUI_T(KC_F20)

/*
 * Right-hand function-layer mod-tap aliases.
 *
 * Keep these in the same left-to-right order as the layer row they occupy.
 */
#define _GUI_F4 RGUI_T(KC_F4)
#define _CTL_F5 RCTL_T(KC_F5)
#define _OPT_F6 RALT_T(KC_F6)
#define _SFT_F11 RSFT_T(KC_F11)

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

/*
 * Base typing layer.
 *
 * The actual alpha bindings mostly live in layout_positions.h so changes to a
 * physical position can be made once and then reused across combos and layers.
 * That includes the refined thumb cluster, where Esc moved to the left macro
 * thumb, Enter moved to the right function thumb, and Backspace now lives on
 * the right navigation thumb.
 */
#define LAYOUT_LAYER_BASE                                                                                                               \
    _L15,       _L14,       _L13,       _L12,       _L11,               _R11,       _R12,       _R13,       _R14,       _R15,           \
    _L25,       _L24,       _L23,       _L22,       _L21,               _R21,       _R22,       _R23,       _R24,       _R25,           \
    _L35,       _L34,       _L33,       _L32,       _L31,               _R31,       _R32,       _R33,       _R34,       _R35,           \
                            _L43,       _L42,       _L41,               _R41,       _R42,       _R43

/*
 * Smart temporary number-entry layer.
 *
 * NumWord is a sparse overlay for inline numeric bursts. The right hand gets
 * the digit cluster, the refined thumbs keep Space / Backspace / Enter on tap,
 * and untouched positions stay transparent so a word-breaking key can fall
 * through to the base layer and switch NumWord off in one press.
 */
#define LAYOUT_LAYER_NUMWORD                                                                                                            \
    _______,    _______,    _______,    _______,    _______,            _______,    FI_7,       FI_8,       FI_9,       _______,        \
    _______,    _______,    _______,    _______,    _______,            _______,    FI_4,       FI_5,       FI_6,       FI_0,           \
    _______,    _______,    _______,    _______,    _______,            _______,    FI_1,       FI_2,       FI_3,       _______,        \
                            KC_SPC,     KC_BSPC,    NUMLOCK,            NUMLOCK,    KC_DEL,     KC_ENT

/*
 * Number-entry layer.
 *
 * This is the full dedicated numeric workspace: the right hand carries the
 * digit cluster while the left hand keeps the base-layer modifier chord
 * positions for Shift / Alt / Ctrl / Gui and Meh / Hyper. QK_LLCK keeps the
 * layer latched until it is pressed again or QMK's layer-lock timeout expires.
 */
#define LAYOUT_LAYER_NUMBERS                                                                                                            \
    FI_DOT,     FI_COMM,    FI_MINS,    FI_PLUS,    FI_SLSH,            XXXXXXX,    FI_7,       FI_8,       FI_9,       XXXXXXX,        \
    KC_LSFT,    KC_RALT,    KC_LCTL,    KC_LGUI,    KC_DEL,             KC_BSPC,    FI_4,       FI_5,       FI_6,       FI_0,           \
    FI_PERC,    FI_LPRN,    KC_MEH,     KC_HYPR,    FI_RPRN,            XXXXXXX,    FI_1,       FI_2,       FI_3,       XXXXXXX,        \
                            KC_SPC,     XXXXXXX,    QK_LLCK,            QK_LLCK,    KC_ENT,     XXXXXXX

/*
 * Navigation layer.
 *
 * Intended for cursor movement and document navigation without leaving the home
 * block. The left hand keeps one-shot variants of the usual modifier block,
 * including Meh and Hyper, so selection and shortcut chords can be queued
 * before a movement key. The right hand keeps plain HJKL arrows for direct
 * cursoring. The tap Backspace on the base thumb remains available through the
 * transparent thumb positions when Navigation is not latched.
 */
#define LAYOUT_LAYER_NAVIGATION                                                                                                         \
    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,            XXXXXXX,    KC_PGDN,    KC_PGUP,    XXXXXXX,    XXXXXXX,        \
    _OSM_LSFT,  _OSM_LALT,  _OSM_LCTL,  _OSM_LGUI,  KC_DEL,             KC_LEFT,    KC_DOWN,    KC_UP,      KC_RGHT,    XXXXXXX,              \
    XXXXXXX,    XXXXXXX,    _OSM_MEH,   _OSM_HYPR,  XXXXXXX,            XXXXXXX,    KC_HOME,    KC_INS,     KC_END,     XXXXXXX,        \
                            _______,    XXXXXXX,    QK_LLCK,            QK_LLCK,    KC_ENT,     XXXXXXX

/*
 * Function-key layer.
 *
 * The refined right thumb makes Enter the tap action for the same key that
 * holds this layer, so the most common confirmation key stays under the
 * stronger thumb while function access remains deliberate.
 */
#define LAYOUT_LAYER_FUNCTION                                                                                                           \
    KC_F21,     KC_F22,     KC_F23,     KC_F24,     KC_PSCR,            KC_PSCR,    KC_F7,      KC_F8,      KC_F9,      KC_F12,         \
    _SFT_F17,   _OPT_F18,   _CTL_F19,   _GUI_F20,   KC_SCRL,            KC_SCRL,    _GUI_F4,    _CTL_F5,    _OPT_F6,    _SFT_F11,       \
    KC_F13,     KC_F14,     KC_F15,     KC_F16,     KC_PAUS,            KC_PAUS,    KC_F1,      KC_F2,      KC_F3,      KC_F10,         \
                            XXXXXXX,    XXXXXXX,    XXXXXXX,            _______,    XXXXXXX,    XXXXXXX

/*
 * Coding-symbol layer.
 *
 * This layer groups the high-frequency punctuation used for programming and
 * structured text so those symbols no longer depend on same-row combos. OS-
 * specific bracket, angle-bracket, backslash, pipe, and similar symbols route
 * through shared userspace custom keycodes.
 *
 * Finnish on macOS is the main reason this exists: several symbols that are
 * straightforward AltGr outputs on PC use different Option/Shift chords on the
 * Mac Finnish layout. Keep those quirks in fleetsing_symbol_process_record()
 * instead of sprinkling raw Mac-specific keycodes through the layer matrix.
 */
#define LAYOUT_LAYER_SYMBOLS                                                                                                            \
    SYM_LBRC,   SYM_LCBR,   FI_LPRN,    SYM_LABK,   FI_EQL,             FI_PLUS,    SYM_RABK,   FI_RPRN,    SYM_RCBR,   SYM_RBRC,       \
    FI_EXLM,    SYM_AT,     FI_HASH,    SYM_DLR,    FI_PERC,            FI_CIRC,    FI_AMPR,    SYM_PIPE,   FI_COLN,    FI_SCLN,        \
    FI_QUOT,    FI_DQUO,    FI_GRV,     SYM_TILD,   SYM_BSLS,           FI_SLSH,    FI_MINS,    FI_UNDS,    FI_QUES,    FI_ASTR,        \
                            KC_SPC,     KC_BSPC,    QK_LLCK,            QK_LLCK,    KC_DEL,     KC_ENT

/*
 * Media/system layer.
 *
 * OS_MAC and OS_PC toggle the persisted Ctrl/GUI swap used for macOS vs PC
 * shortcut ergonomics. Symbols still follow the Finnish OS layout in both
 * modes; only the modifier behavior and mode-dependent symbol chords change.
 *
 * This layer keeps OS-mode toggles and transport/media controls. Maintenance
 * actions stay off this layer so it remains safe to lock and use casually.
 */
#define LAYOUT_LAYER_MEDIA                                                                                                              \
    OS_MAC,     OS_PC,      XXXXXXX,    XXXXXXX,    XXXXXXX,            XXXXXXX,    XXXXXXX,    KC_UP,      XXXXXXX,    XXXXXXX,        \
    KC_MPRV,    KC_VOLD,    KC_MUTE,    KC_VOLU,    KC_MNXT,            XXXXXXX,    KC_LEFT,    KC_DOWN,    KC_RGHT,    XXXXXXX,        \
    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,            XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,        \
                            KC_MPLY,    XXXXXXX,    KC_MSTP,            XXXXXXX,    XXXXXXX,    XXXXXXX

/*
 * Pointer and mouse-button layer.
 *
 * This layer does not choose the active sensor DPI directly. It toggles
 * Charybdis sniping behavior and exposes sniping-DPI controls while userspace
 * decides which sensor's motion is converted into scroll.
 *
 * The mirrored outer-corner boot keys are guarded so transient pointer-layer
 * entry cannot trigger the bootloader accidentally.
 */
#define LAYOUT_LAYER_POINTER                                                                                                            \
    /* Pointer-layer DPI controls are sniping-DPI controls because this layer auto-enables sniping mode. */                             \
    BOOT_SAFE,  XXXXXXX,    XXXXXXX,    S_D_RMOD,   S_D_MOD,            S_D_MOD,    S_D_RMOD,   XXXXXXX,    XXXXXXX,    BOOT_SAFE,      \
    KC_LSFT,    KC_RALT,    KC_LCTL,    KC_LGUI,    XXXXXXX,            XXXXXXX,    KC_RGUI,    KC_RCTL,    KC_RALT,    KC_RSFT,        \
    _______,    DRGSCRL,    XXXXXXX,    SET_MS_L,   XXXXXXX,            XXXXXXX,    SET_MS_R,   XXXXXXX,    DRGSCRL,    _______,        \
                            MS_BTN2,    MS_BTN1,    QK_LLCK,            QK_LLCK,    MS_BTN1,    MS_BTN2

/*
 * Dynamic-macro and desktop-shortcut layer.
 *
 * Esc moved off this hold-tap and onto the opposite inner thumb, so this layer
 * now uses the left inner thumb purely as "Esc on tap, Macro on hold".
 */
#define LAYOUT_LAYER_MACRO                                                                                                              \
    XXXXXXX,    XXXXXXX,    DM_REC2,    DM_REC1,    XXXXXXX,            XXXXXXX,    XXXXXXX,    KC_UP,      XXXXXXX,    XXXXXXX,        \
    XXXXXXX,    XXXXXXX,    DM_PLY2,    DM_PLY1,    DM_RSTP,            XXXXXXX,    KC_LEFT,    KC_DOWN,    KC_RGHT,    XXXXXXX,        \
    XXXXXXX,    G(FI_Z),    G(FI_X),    G(FI_C),    G(FI_V),            XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,    XXXXXXX,        \
                            XXXXXXX,    XXXXXXX,    QK_LLCK,            QK_LLCK,    XXXXXXX,    XXXXXXX

/* Wrap the board's LAYOUT macro so layer macros stay visually grouped. */
#define LAYOUT_wrapper(...) LAYOUT(__VA_ARGS__)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [LAYER_BASE]       = LAYOUT_wrapper(LAYOUT_LAYER_BASE),
    [LAYER_NUMWORD]    = LAYOUT_wrapper(LAYOUT_LAYER_NUMWORD),
    [LAYER_NUMBERS]    = LAYOUT_wrapper(LAYOUT_LAYER_NUMBERS),
    [LAYER_NAVIGATION] = LAYOUT_wrapper(LAYOUT_LAYER_NAVIGATION),
    [LAYER_FUNCTION]   = LAYOUT_wrapper(LAYOUT_LAYER_FUNCTION),
    [LAYER_SYMBOLS]    = LAYOUT_wrapper(LAYOUT_LAYER_SYMBOLS),
    [LAYER_MEDIA]      = LAYOUT_wrapper(LAYOUT_LAYER_MEDIA),
    [LAYER_POINTER]    = LAYOUT_wrapper(LAYOUT_LAYER_POINTER),
    [LAYER_MACRO]      = LAYOUT_wrapper(LAYOUT_LAYER_MACRO),
};

/*
 * Dual-role timing is tuned by key role instead of one global term:
 * - home-row mods stay short to reduce accidental holds while typing
 * - lower-row modifier taps get a little more time for outward reaches
 * - thumb layer-taps stay longest so Space/Tab/Enter/Backspace taps
 *   remain easy while still making held layers feel deliberate
 *
 * Most cases are still keyed by physical position. When thumb roles move,
 * revisit the thumb cases below so the intended feel follows the role swap.
 */
uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
    (void)record;

    switch (keycode) {
        case _L25:
        case _L24:
        case _L23:
        case _L22:
        case _R22:
        case _R23:
        case _R24:
        case _R25:
            return 175;

        case _L35:
        case _L33:
        case _L32:
        case _R35:
        case _R33:
        case _R32:
            return 190;

        case _R41:
        case _R42:
            return 215;

        case _L41:
        case _L43:
        case _R43:
            return 230;

        case _SFT_F17:
        case _OPT_F18:
        case _CTL_F19:
        case _GUI_F20:
        case _GUI_F4:
        case _CTL_F5:
        case _OPT_F6:
        case _SFT_F11:
            return 175;

        default:
            return TAPPING_TERM;
    }
}

/*
 * Keep permissive-hold enabled on non-thumb hold-taps, but make the thumb
 * layer-taps less eager to resolve as hold during rolling input.
 */
bool get_permissive_hold(uint16_t keycode, keyrecord_t *record) {
    (void)record;

    switch (keycode) {
        case _L41:
        case _L43:
        case _R41:
        case _R42:
        case _R43:
            return false;
        default:
            return true;
    }
}

/*
 * NumWord borrows the base-layer positional combos so the same physical thumb
 * and symbol chords keep working while the sparse overlay is active.
 */
uint8_t combo_ref_from_layer(uint8_t layer) {
    switch (layer) {
        case LAYER_NUMWORD:
        case LAYER_NUMBERS:
            return LAYER_BASE;
        default:
            return layer;
    }
}

/*
 * Thumb combos stay intentionally tighter than the vertical symbol chords so
 * repeat keys and pointer buttons do not fire during normal rolling input.
 * Caps Word and the cross-thumb NumWord combo get a wider term because they
 * span both hands and are meant to feel deliberate.
 */
uint16_t get_combo_term(uint16_t combo_index, combo_t *combo) {
    (void)combo;

    switch (combo_index) {
        case L42_L41:
        case L43_L41:
        case L43_L42:
        case R41_R43:
            return 52;

        case L41_R41:
            return 80;

        case L11_L21:
        case L21_L31:
        case L12_L22:
        case L22_L32:
        case L13_L23:
        case L23_L33:
        case L14_L24:
        case L24_L34:
        case R11_R21:
        case R21_R31:
        case R12_R22:
        case R22_R32:
        case R13_R23:
        case R23_R33:
        case R14_R24:
        case R24_R34:
        case R25_R35:
            return 70;

        case L43_R43:
            return 80;
        default:
            return COMBO_TERM;
    }
}

/*
 * Cross-thumb stateful combos should resolve only from taps. That avoids
 * fighting the held layer-taps that now own Esc, Enter, and Backspace.
 */
bool get_combo_must_tap(uint16_t combo_index, combo_t *combo) {
    (void)combo;

    switch (combo_index) {
        case L41_R41:
        case L43_R43:
            return true;
        default:
            return false;
    }
}
// clang-format on

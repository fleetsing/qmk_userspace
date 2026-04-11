#pragma once

/*
 * Positional aliases for the 3x5 Charybdis layout.
 *
 * Naming convention:
 * - `_Lxy` and `_Rxy` are left/right physical positions
 * - `x` is the row number, `y` is the position within that side
 *
 * Keep physical-position behavior here so combos and the base layer can refer
 * to the same positions without duplicating layer-tap or mod-tap details.
 */

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
// LT(layer, key) taps as key and holds as the target layer.
#define _L43 LT(LAYER_NUMBERS, KC_SPC)
#define _L42 MS_BTN1
#define _L41 LT(LAYER_MACRO, KC_BSPC)

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

/*
 * Current thumb roles:
 * - L43 = Space tap, Numbers hold
 * - L42 = Mouse 1
 * - L41 = Backspace tap, Macro hold
 * - R41 = Esc tap, Function hold
 * - R42 = Tab tap, Symbols hold
 * - R43 = Enter tap, Navigation hold
 *
 * Cross-thumb behavior:
 * - L43 + R43 tap = NumWord, hold = Media via tri-layer
 * - L41 + R41 combo = Caps Word
 */
// Right-hand thumb cluster.
#define _R41 LT(LAYER_FUNCTION, KC_ESC)
/* Tap for Tab; hold for the dedicated coding-symbol layer. */
#define _R42 LT(LAYER_SYMBOLS, KC_TAB)
#define _R43 LT(LAYER_NAVIGATION, KC_ENT)

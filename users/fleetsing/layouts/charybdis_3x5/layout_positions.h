#pragma once

/*
 * Positional aliases for the 3x5 Charybdis layout.
 *
 * Naming convention:
 * - `_Lxy` and `_Rxy` are left/right physical positions
 * - `x` is the row number, `y` is the position within that side
 *
 * Section order mirrors the base-layer matrix:
 * - left rows top to bottom, then the left thumb cluster
 * - right rows top to bottom, then the right thumb cluster
 * - each section stays in the same left-to-right order used in keymap.c
 *
 * Keep physical-position behavior here so combos and the base layer can refer
 * to the same positions without duplicating layer-tap or mod-tap details.
 */

/*
 * Left-hand row 1.
 */
#define _L15 FI_W
#define _L14 FI_G
#define _L13 MEH_T(FI_D)
#define _L12 HYPR_T(FI_F)
#define _L11 FI_B

/*
 * Left-hand row 2.
 */
#define _L25 LSFT_T(FI_R)
#define _L24 RALT_T(FI_S)
#define _L23 LCTL_T(FI_T)
#define _L22 LGUI_T(FI_N)
#define _L21 FI_K

/*
 * Left-hand row 3.
 */
#define _L35 LT(LAYER_POINTER, FI_X)
#define _L34 FI_C
#define _L33 LT(LAYER_SCROLL_LEFT, FI_M)
#define _L32 FI_P
#define _L31 FI_V

/*
 * Left-hand thumb cluster.
 *
 * Keep the roles listed in the same left-to-right order as the matrix:
 * - L43 = Space tap, Navigation hold
 * - L42 = Mouse 1
 * - L41 = Esc tap, Numbers hold
 */
#define _L43 LT(LAYER_NAVIGATION, KC_SPC)
#define _L42 MS_BTN1
#define _L41 LT(LAYER_NUMBERS, KC_ESC)

/*
 * Right-hand row 1.
 */
#define _R11 FI_Z
#define _R12 HYPR_T(FI_H)
#define _R13 MEH_T(FI_U)
#define _R14 FI_O
#define _R15 FI_Y

/*
 * Right-hand row 2.
 */
#define _R21 FI_J
#define _R22 RGUI_T(FI_L)
#define _R23 RCTL_T(FI_E)
#define _R24 RALT_T(FI_A)
#define _R25 RSFT_T(FI_I)

/*
 * Right-hand row 3.
 */
#define _R31 FI_DOT
#define _R32 FI_COMM
#define _R33 LT(LAYER_SCROLL_RIGHT, FI_ADIA)
#define _R34 FI_ODIA
#define _R35 LT(LAYER_POINTER, FI_Q)

/*
 * Right-hand thumb cluster.
 *
 * The refined thumb layout keeps the same "editing tap on the inner thumb,
 * layer hold on the same key" pattern used on the left side:
 * - R41 = Enter tap, Media hold
 * - R42 = Tab tap, Function hold
 * - R43 = Backspace tap, Navigation hold
 */
#define _R41 LT(LAYER_MEDIA, KC_ENT)
#define _R42 LT(LAYER_FUNCTION, KC_TAB)
#define _R43 LT(LAYER_NAVIGATION, KC_BSPC)

/*
 * Cross-thumb behaviors.
 *
 * These combos are named by physical positions in combos.def so they stay
 * stable even if the tap/hold roles on those thumbs are adjusted:
 * - L43 + R43 taps into NumWord; holding both Navigation thumbs promotes to
 *   Symbols
 * - L41 + R41 taps together for Caps Word on the Esc + Enter pair
 * - L41 + R41 holds promote to Macro through the Numbers + Media tri-layer
 */

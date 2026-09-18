#pragma once

/*
 * QWERTY test variant of the positional aliases for the 3x5 Charybdis layout.
 *
 * This mirrors layout_positions.h exactly in structure: every mod-tap,
 * layer-tap, and thumb role stays pinned to the same physical position. Only
 * the base letter/symbol assigned to each position changes, to a standard
 * QWERTY arrangement adapted to fit 30 physical slots (26 letters + comma +
 * dot + A with diaeresis + O with diaeresis).
 *
 * Deliberate deviations from a textbook QWERTY layout, to keep comma, dot,
 * and the two Finnish letters reachable on a 3x5 grid:
 * - Right-hand row 2 has 5 slots instead of QWERTY's 4 (H J K L), so O with
 *   diaeresis takes the 5th slot, inheriting that position's Shift-hold role.
 * - Right-hand row 3 holds N, M, comma, dot, A-with-diaeresis (in that
 *   column order) instead of N, M, comma, dot, slash, so all of comma, dot,
 *   and A-with-diaeresis stay reachable without a 4th row.
 *
 * See layout_positions.h for the full naming convention and section-order
 * notes; they apply identically here.
 */

/*
 * Left-hand row 1.
 */
#define _L15 FI_Q
#define _L14 FI_W
#define _L13 FI_E
#define _L12 FI_R
#define _L11 FI_T

/*
 * Left-hand row 2.
 */
#define _L25 LSFT_T(FI_A)
#define _L24 RALT_T(FI_S)
#define _L23 LCTL_T(FI_D)
#define _L22 LGUI_T(FI_F)
#define _L21 FI_G

/*
 * Left-hand row 3.
 */
#define _L35 LT(LAYER_POINTER, FI_Z)
#define _L34 FI_X
#define _L33 MEH_T(FI_C)
#define _L32 HYPR_T(FI_V)
#define _L31 FI_B

/*
 * Left-hand thumb cluster. Unchanged from layout_positions.h.
 */
#define _L43 LT(LAYER_NAVIGATION, KC_SPC)
#define _L42 MS_BTN1
#define _L41 LT(LAYER_NUMBERS, KC_ESC)

/*
 * Right-hand row 1.
 */
#define _R11 FI_Y
#define _R12 FI_U
#define _R13 FI_I
#define _R14 FI_O
#define _R15 FI_P

/*
 * Right-hand row 2.
 *
 * O-with-diaeresis takes the 5th slot after L (its physical neighbor on a
 * real Finnish keyboard row), keeping that position's Shift-hold role.
 */
#define _R21 FI_H
#define _R22 RGUI_T(FI_J)
#define _R23 RCTL_T(FI_K)
#define _R24 RALT_T(FI_L)
#define _R25 RSFT_T(FI_ODIA)

/*
 * Right-hand row 3.
 *
 * N, M, comma, dot, A-with-diaeresis, per request: comma keeps this row's
 * Meh-hold role and A-with-diaeresis keeps this row's Pointer layer-tap role.
 */
#define _R31 FI_N
#define _R32 HYPR_T(FI_M)
#define _R33 MEH_T(FI_COMM)
#define _R34 FI_DOT
#define _R35 LT(LAYER_POINTER, FI_ADIA)

/*
 * Right-hand thumb cluster. Unchanged from layout_positions.h.
 */
#define _R41 LT(LAYER_MEDIA, KC_ENT)
#define _R42 LT(LAYER_FUNCTION, KC_TAB)
#define _R43 LT(LAYER_NAVIGATION, KC_BSPC)

/*
 * Indirection for the dot/comma custom Auto Shift symbol overrides.
 *
 * Comma and dot moved to _R33 and _R34 in this variant; point the shared
 * indirection macros at their new positions so fi_autoshift.c keeps applying
 * the "!"/"?" shifted-symbol overrides to the right keys. See
 * layout_positions.h for the full rationale.
 */
#define _FLEETSING_DOT_POS   _R34
#define _FLEETSING_COMMA_POS _R33

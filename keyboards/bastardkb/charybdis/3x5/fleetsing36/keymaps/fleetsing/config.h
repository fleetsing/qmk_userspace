/**
 * Copyright 2021 Charly Delay <charly@codesink.dev> (@0xcharly)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#ifdef VIA_ENABLE
/* VIA configuration. */
#    define DYNAMIC_KEYMAP_LAYER_COUNT 7
#endif // VIA_ENABLE

#ifndef __arm__
/* Disable unused features. */
#    define NO_ACTION_ONESHOT
#endif // __arm__

#define RGB_MATRIX_DISABLE_SHARED_KEYCODES

/* Layer lock auto-clears after 60 seconds to avoid leaving a layer latched indefinitely. */
#define LAYER_LOCK_IDLE_TIMEOUT 60000 // Turn off after 60 seconds.
/* Global fallback tap-vs-hold threshold for dual-role keys. */
#define TAPPING_TERM 200
/* Most base-layer dual-role keys use shorter or longer per-key tuning. */
#define TAPPING_TERM_PER_KEY

/* Combos must be entered quickly enough to avoid conflicting with normal rolling input. */
#define COMBO_TERM 35
/* Selected combos override the default term or require a tap-only resolution. */
#define COMBO_TERM_PER_COMBO
#define COMBO_MUST_TAP_PER_COMBO

/* Reserve userspace split-RPC ids for synced OLED-facing runtime state. */
#define SPLIT_TRANSACTION_IDS_USER RPC_ID_USER_NUMWORD_SYNC, RPC_ID_USER_DISPLAY_SYNC, RPC_ID_USER_LEFT_KNOB_STATE
/*
 * The OLED display-sync payload includes several short overlay items, which is
 * larger than QMK's default 32-byte split RPC buffer.
 */
#define RPC_M2S_BUFFER_SIZE 96
#define RPC_S2M_BUFFER_SIZE 96

/*
 * NumWord is a smart temporary number-entry mode layered above the base layer.
 * It times out automatically so a forgotten activation does not linger.
 */
#define FLEETSING_NUMWORD_IDLE_TIMEOUT 5000

/*
 * Auto Shift timing:
 * - AUTO_SHIFT_TIMEOUT is the hold duration before shifted output is chosen
 * - RETRO_SHIFT allows the decision to resolve on release
 * - AUTO_SHIFT_REPEAT lets held shifted keys keep repeating
 */
#define AUTO_SHIFT_TIMEOUT 300
#define AUTO_SHIFT_REPEAT
#define NO_AUTO_SHIFT_NUMERIC
#define RETRO_SHIFT 700

/*
 * Hold-tap tuning:
 * - PERMISSIVE_HOLD_PER_KEY allows hold bias to stay on most dual-role keys while thumbs opt out
 * - CHORDAL_HOLD reduces accidental taps in fast multi-key rolls
 * - FLOW_TAP_TERM keeps very fast same-hand rolls feeling like taps
 * - RETRO_TAPPING lets a released hold-tap still count as a tap when appropriate
 */
#define PERMISSIVE_HOLD_PER_KEY
#define CHORDAL_HOLD
#define FLOW_TAP_TERM 150
#define RETRO_TAPPING

/* Split keyboard settings. */
/* Mirror layer state so OLEDs and layer-driven features stay coherent on both halves. */
#define SPLIT_POINTING_ENABLE
#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_MODS_ENABLE
#define SPLIT_WPM_ENABLE
#define SPLIT_OLED_ENABLE
#define SPLIT_HAPTIC_ENABLE
#define SPLIT_ACTIVITY_ENABLE
#define SPLIT_LED_STATE_ENABLE
/* Required so sniping/drag-scroll state is mirrored to the other half. */
#define CHARYBDIS_CONFIG_SYNC

/* Pointing device settings. */
#define PMW33XX_CPI 1600
/*
 * Charybdis stores DPI as step indices in EEPROM. These values define the
 * lowest runtime step, not the only possible DPI:
 * - default DPI uses 16 steps total
 * - sniping DPI uses 4 steps total
 * Keep the minimums aligned with the intended feel, but remember a flashed
 * board may still come up above these floors if EEPROM already stores a higher
 * step.
 */
#define CHARYBDIS_MINIMUM_DEFAULT_DPI PMW33XX_CPI
#define CHARYBDIS_MINIMUM_SNIPING_DPI 600
/* The remaining right-hand trackball uses the stock Charybdis single-sensor path. */

/* Five quick taps on a one-shot key locks it; otherwise one-shot state times out after 3 seconds. */
#define ONESHOT_TAP_TOGGLE 5
#define ONESHOT_TIMEOUT 3000

#ifdef OLED_ENABLE
/* OLED configuration. */
/*
 * OLED profile selection.
 *
 * Keep the profile switch here so trying a different OLED population only
 * requires changing one define.
 *
 * In this workspace, the original BastardKB modules behaved like SH1107
 * 64x128 panels. The current replacement 4-pin modules are confirmed working
 * as SSD1312 128x64, together with the local SSD1312 render-path fixes in the
 * firmware OLED driver.
 */
#    define FLEETSING_OLED_PROFILE_SH1107_64X128 0
#    define FLEETSING_OLED_PROFILE_SH1107_128X128 1
#    define FLEETSING_OLED_PROFILE_SSD1312_128X64 2
#    define FLEETSING_OLED_PROFILE_SSD1306_128X64 3
#    define FLEETSING_OLED_PROFILE_SH1106_128X64 4

/*
 * Some SH1107 modules that share the same physical form factor still differ in
 * how their COM outputs are wired to the glass. Keep this as a second-stage
 * switch so we can try panel-layout variants without changing driver family or
 * geometry.
 */
#    define FLEETSING_OLED_SH1107_LAYOUT_ALT 0
#    define FLEETSING_OLED_SH1107_LAYOUT_ALT_LR 1
#    define FLEETSING_OLED_SH1107_LAYOUT_SEQ 2
#    define FLEETSING_OLED_SH1107_LAYOUT_SEQ_LR 3

/*
 * SSD1312 replacement modules can still differ in scan direction from the
 * original BastardKB parts. Keep explicit flip/rotation switches here so
 * orientation fixes stay in keymap config instead of requiring driver edits.
 */
#    ifndef FLEETSING_OLED_SSD1312_ROTATION_LEFT
#        define FLEETSING_OLED_SSD1312_ROTATION_LEFT OLED_ROTATION_90
#    endif
#    ifndef FLEETSING_OLED_SSD1312_ROTATION_RIGHT
#        define FLEETSING_OLED_SSD1312_ROTATION_RIGHT OLED_ROTATION_90
#    endif
#    ifndef FLEETSING_OLED_SSD1312_FLIP_SEGMENT
#        define FLEETSING_OLED_SSD1312_FLIP_SEGMENT 0
#    endif
#    ifndef FLEETSING_OLED_SSD1312_FLIP_COM
#        define FLEETSING_OLED_SSD1312_FLIP_COM 0
#    endif

#    ifndef FLEETSING_OLED_PROFILE
/* Current confirmed default for the replacement OLED batch. */
#        define FLEETSING_OLED_PROFILE FLEETSING_OLED_PROFILE_SSD1312_128X64
#    endif

#    ifndef FLEETSING_OLED_SH1107_LAYOUT
/* Next high-probability guess after mirrored-looking SH1107 output. */
#        define FLEETSING_OLED_SH1107_LAYOUT FLEETSING_OLED_SH1107_LAYOUT_ALT_LR
#    endif

#    if FLEETSING_OLED_SH1107_LAYOUT == FLEETSING_OLED_SH1107_LAYOUT_ALT
#        define FLEETSING_OLED_SH1107_COM_PINS COM_PINS_ALT
#    elif FLEETSING_OLED_SH1107_LAYOUT == FLEETSING_OLED_SH1107_LAYOUT_ALT_LR
#        define FLEETSING_OLED_SH1107_COM_PINS COM_PINS_ALT_LR
#    elif FLEETSING_OLED_SH1107_LAYOUT == FLEETSING_OLED_SH1107_LAYOUT_SEQ
#        define FLEETSING_OLED_SH1107_COM_PINS COM_PINS_SEQ
#    elif FLEETSING_OLED_SH1107_LAYOUT == FLEETSING_OLED_SH1107_LAYOUT_SEQ_LR
#        define FLEETSING_OLED_SH1107_COM_PINS COM_PINS_SEQ_LR
#    else
#        error "Unsupported FLEETSING_OLED_SH1107_LAYOUT"
#    endif

#    if FLEETSING_OLED_SSD1312_FLIP_SEGMENT
#        define OLED_FLIP_SEGMENT
#    endif
#    if FLEETSING_OLED_SSD1312_FLIP_COM
#        define OLED_FLIP_COM
#    endif

#    if FLEETSING_OLED_PROFILE == FLEETSING_OLED_PROFILE_SH1107_64X128
#        define OLED_IC OLED_IC_SH1107
#        define OLED_DISPLAY_64X128
#        define OLED_COM_PINS FLEETSING_OLED_SH1107_COM_PINS
#        define FLEETSING_OLED_ROTATION_LEFT OLED_ROTATION_270
#        define FLEETSING_OLED_ROTATION_RIGHT OLED_ROTATION_180
#    elif FLEETSING_OLED_PROFILE == FLEETSING_OLED_PROFILE_SH1107_128X128
#        define OLED_IC OLED_IC_SH1107
#        define OLED_DISPLAY_128X128
#        define OLED_COM_PINS FLEETSING_OLED_SH1107_COM_PINS
#        define FLEETSING_OLED_ROTATION_LEFT OLED_ROTATION_270
#        define FLEETSING_OLED_ROTATION_RIGHT OLED_ROTATION_180
#    elif FLEETSING_OLED_PROFILE == FLEETSING_OLED_PROFILE_SSD1312_128X64
#        define OLED_IC OLED_IC_SSD1312
#        define OLED_DISPLAY_128X64
#        define FLEETSING_OLED_ROTATION_LEFT FLEETSING_OLED_SSD1312_ROTATION_LEFT
#        define FLEETSING_OLED_ROTATION_RIGHT FLEETSING_OLED_SSD1312_ROTATION_RIGHT
#    elif FLEETSING_OLED_PROFILE == FLEETSING_OLED_PROFILE_SSD1306_128X64
#        define OLED_IC OLED_IC_SSD1306
#        define OLED_DISPLAY_128X64
#        define FLEETSING_OLED_ROTATION_LEFT OLED_ROTATION_180
#        define FLEETSING_OLED_ROTATION_RIGHT OLED_ROTATION_180
#    elif FLEETSING_OLED_PROFILE == FLEETSING_OLED_PROFILE_SH1106_128X64
#        define OLED_IC OLED_IC_SH1106
#        define OLED_DISPLAY_128X64
#        define FLEETSING_OLED_ROTATION_LEFT OLED_ROTATION_180
#        define FLEETSING_OLED_ROTATION_RIGHT OLED_ROTATION_180
#    else
#        error "Unsupported FLEETSING_OLED_PROFILE"
#    endif
/*
 * Keep QMK's built-in timeout disabled for this keymap.
 *
 * With split pointing enabled, tiny continuous pointer activity can keep the
 * core timeout from ever appearing idle. Userspace handles OLED sleep/wake
 * explicitly instead, using key and trackball activity that is filtered for
 * real user input.
 */
#    define OLED_TIMEOUT 0
/*
 * Sleep the OLEDs after 30 seconds without tracked key or pointing activity.
 *
 * The final implementation uses a hard oled_off() on the master half. Fade-out
 * was tested and rejected because it did not behave reliably on this SH1107
 * path, while hard-off does.
 */
#    define FLEETSING_OLED_IDLE_TIMEOUT 30000
#endif // OLED_ENABLE

#ifdef HAPTIC_ENABLE
/* Haptic configuration. */
/* These DRV2605L values are board-specific tuning for the installed actuator. */
#    define DRV2605L_FB_ERM_LRA 0
#    define DRV2605L_FB_BRAKEFACTOR 3 /* For 1x:0, 2x:1, 3x:2, 4x:3, 6x:4, 8x:5, 16x:6, Disable Braking:7 */
#    define DRV2605L_FB_LOOPGAIN 1    /* For  Low:0, Medium:1, High:2, Very High:3 */
#    define DRV2605L_RATED_VOLTAGE 3
#    define DRV2605L_V_PEAK 5
#    define DRV2605L_GREETING 21
/* keyboard_post_init_user() restores this mode if EEPROM or runtime state changed it. */
#    define DRV2605L_DEFAULT_MODE 21
/* Category filters for users/fleetsing/haptics.c. */
#    define NO_HAPTIC_ALPHA
#    define NO_HAPTIC_NUMERIC
#    define NO_HAPTIC_PUNCTUATION
#    define NO_HAPTIC_NAV
#    define NO_HAPTIC_MOD
#endif // HAPTIC_ENABLE

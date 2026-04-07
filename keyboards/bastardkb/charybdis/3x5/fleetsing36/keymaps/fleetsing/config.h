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

#define LAYER_LOCK_IDLE_TIMEOUT 60000 // Turn off after 60 seconds.
#define TAPPING_TERM 300

#define COMBO_TERM 35

#define CAPS_WORD_IDLE_TIMEOUT 3000
#define AUTO_SHIFT_TIMEOUT 300
#define AUTO_SHIFT_REPEAT
#define NO_AUTO_SHIFT_NUMERIC
#define RETRO_SHIFT 700

#define PERMISSIVE_HOLD
#define CHORDAL_HOLD
#define FLOW_TAP_TERM 150
#define RETRO_TAPPING

/* Split keyboard settings. */
#define SPLIT_POINTING_ENABLE
#define SPLIT_LAYER_STATE_ENABLE
#define SPLIT_MODS_ENABLE
#define SPLIT_WPM_ENABLE
#define SPLIT_OLED_ENABLE
#define SPLIT_HAPTIC_ENABLE
#define SPLIT_ACTIVITY_ENABLE
#define SPLIT_LED_STATE_ENABLE
#define CHARYBDIS_CONFIG_SYNC

/* Pointing device settings. */
#define PMW33XX_CPI 1600
#undef POINTING_DEVICE_RIGHT
#define POINTING_DEVICE_COMBINED

#define POINTING_DEVICE_INVERT_X_RIGHT
#define POINTING_DEVICE_ROTATION_90

#define ONESHOT_TAP_TOGGLE 5
#define ONESHOT_TIMEOUT 3000

#ifdef OLED_ENABLE
/* OLED configuration. */
#    define OLED_IC OLED_IC_SH1107
#    define OLED_DISPLAY_64X128
#endif // OLED_ENABLE

#ifdef HAPTIC_ENABLE
/* Haptic configuration. */
#    define DRV2605L_FB_ERM_LRA 0
#    define DRV2605L_FB_BRAKEFACTOR 3 /* For 1x:0, 2x:1, 3x:2, 4x:3, 6x:4, 8x:5, 16x:6, Disable Braking:7 */
#    define DRV2605L_FB_LOOPGAIN 1    /* For  Low:0, Medium:1, High:2, Very High:3 */
#    define DRV2605L_RATED_VOLTAGE 3
#    define DRV2605L_V_PEAK 5
#    define DRV2605L_GREETING 21
#    define DRV2605L_DEFAULT_MODE 21
#    define NO_HAPTIC_ALPHA
#    define NO_HAPTIC_NUMERIC
#    define NO_HAPTIC_PUNCTUATION
#    define NO_HAPTIC_NAV
#    define NO_HAPTIC_MOD
#endif // HAPTIC_ENABLE

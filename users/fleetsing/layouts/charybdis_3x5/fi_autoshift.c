#include "fleetsing.h"
#include "layouts/charybdis_3x5/layout_positions.h"

/*
 * Retro Shift resolves on release, so this state tracks a pending key long
 * enough to fire a haptic cue once the Auto Shift timeout has elapsed.
 */
static struct {
    bool        active;
    bool        fired;
    uint16_t    keycode;
    uint16_t    start_time;
    keyrecord_t record;
} fleetsing_autoshift_haptic_state = {0};

static void fleetsing_autoshift_resolution_haptic(bool shifted, keyrecord_t *record) {
#ifdef HAPTIC_ENABLE
    if (shifted && record->event.pressed && record->event.time == 0 && haptic_get_enable()) {
        haptic_play();
    }
#endif
}

/*
 * Finnish symbol access keeps the same physical combos in both OS modes.
 *
 * Some symbols are raw Finnish-layout keycodes on both platforms, while others
 * need different modifier chords on macOS and PC even though the resulting
 * glyph should stay the same.
 *
 * Keep the combo outputs themselves on their original raw keycodes. That
 * preserves the Retro Shift behavior that already worked before OS-mode
 * support was added. OS-specific branching only happens here when the actual
 * emitted symbol differs between macOS and PC mode.
 *
 * Angle brackets are the main example: macOS intentionally keeps the original
 * working grave-key behavior (`FI_SECT` / `S(FI_SECT)`), while PC mode uses
 * the Finnish angle-bracket keycodes instead.
 */
static uint16_t fleetsing_autoshift_output_keycode(uint16_t keycode, bool shifted) {
    switch (keycode) {
        case S(FI_4):
            return shifted ? fleetsing_os_keycode(A(FI_4), FI_DLR) : S(FI_4);
        case A(FI_2):
            return fleetsing_os_keycode(A(FI_2), FI_AT);
        case FI_DOT:
            return shifted ? S(FI_PLUS) : FI_DOT;
        case FI_COMM:
            return shifted ? S(FI_1) : FI_COMM;
        case FI_PLUS:
            return shifted ? S(FI_0) : FI_PLUS;
        case FI_MINS:
            return shifted ? S(FI_MINS) : FI_MINS;
        case S(FI_7):
            return shifted ? fleetsing_os_keycode(S(A(FI_7)), FI_BSLS) : S(FI_7);
        case S(FI_QUOT):
            return shifted ? S(FI_3) : S(FI_QUOT);
        case S(FI_DOT):
            return shifted ? S(FI_COMM) : S(FI_DOT);
        case A(FI_DIAE):
            return fleetsing_os_keycode(A(FI_DIAE), FI_TILD);
        case S(FI_8):
            return shifted ? fleetsing_os_keycode(S(A(FI_8)), FI_LCBR) : S(FI_8);
        case A(FI_8):
            return shifted ? fleetsing_os_keycode(FI_SECT, FI_LABK) : fleetsing_os_keycode(A(FI_8), FI_LBRC);
        case S(FI_9):
            return shifted ? fleetsing_os_keycode(S(A(FI_9)), FI_RCBR) : S(FI_9);
        case A(FI_9):
            return shifted ? fleetsing_os_keycode(S(FI_SECT), FI_RABK) : fleetsing_os_keycode(A(FI_9), FI_RBRC);
        case FI_QUOT:
            return shifted ? S(FI_2) : FI_QUOT;
        case S(FI_6):
            return shifted ? S(FI_5) : S(FI_6);
        default:
            return (IS_RETRO(keycode)) ? keycode & 0xFF : keycode;
    }
}

bool get_custom_auto_shifted_key(uint16_t keycode, keyrecord_t *record) {
    /*
     * Keep this list in sync with the explicit custom mappings below in
     * autoshift_press_user() and autoshift_release_user(). If a key is added
     * here without a matching press/release mapping, it falls back to normal
     * Shift behavior instead of the intended Finnish symbol override.
     */
    switch (keycode) {
        case S(FI_4):
        case A(FI_2):
        case FI_DOT:
        case FI_COMM:
        case FI_PLUS:
        case FI_MINS:
        case S(FI_7):
        case S(FI_QUOT):
        case S(FI_DOT):
        case A(FI_DIAE):
        case S(FI_8):
        case A(FI_8):
        case S(FI_9):
        case A(FI_9):
        case FI_QUOT:
        case S(FI_6):
        case _L25:
        case _L24:
        case _L23:
        case _L22:
        case _L32:
        case _L33:
        case _R35:
        case _R33:
        case _R32:
        case _R22:
        case _R23:
        case _R24:
        case _R25:
            return true;
        default:
            return false;
    }
}

static bool fleetsing_is_retro_autoshift_key(uint16_t keycode, keyrecord_t *record) {
    return IS_RETRO(keycode) && get_custom_auto_shifted_key(keycode, record);
}

static void fleetsing_clear_autoshift_haptic_state(void) {
    fleetsing_autoshift_haptic_state.active  = false;
    fleetsing_autoshift_haptic_state.fired   = false;
    fleetsing_autoshift_haptic_state.keycode = KC_NO;
}

bool fleetsing_autoshift_haptic_process_record(uint16_t keycode, keyrecord_t *record) {
    if (fleetsing_autoshift_haptic_state.active && record->event.pressed && keycode != fleetsing_autoshift_haptic_state.keycode) {
        fleetsing_clear_autoshift_haptic_state();
    }

    if (!fleetsing_is_retro_autoshift_key(keycode, record)) {
        return true;
    }

    if (record->event.pressed) {
        fleetsing_autoshift_haptic_state.active     = true;
        fleetsing_autoshift_haptic_state.fired      = false;
        fleetsing_autoshift_haptic_state.keycode    = keycode;
        fleetsing_autoshift_haptic_state.start_time = timer_read();
        fleetsing_autoshift_haptic_state.record     = *record;
    } else if (fleetsing_autoshift_haptic_state.active && keycode == fleetsing_autoshift_haptic_state.keycode) {
        fleetsing_clear_autoshift_haptic_state();
    }

    return true;
}

void fleetsing_autoshift_haptic_matrix_scan(void) {
#ifdef HAPTIC_ENABLE
    if (!fleetsing_autoshift_haptic_state.active || fleetsing_autoshift_haptic_state.fired || !haptic_get_enable()) {
        return;
    }

    if (TIMER_DIFF_16(timer_read(), fleetsing_autoshift_haptic_state.start_time) >= get_autoshift_timeout(fleetsing_autoshift_haptic_state.keycode, &fleetsing_autoshift_haptic_state.record)) {
        haptic_play();
        fleetsing_autoshift_haptic_state.fired = true;
    }
#endif
}

void autoshift_press_user(uint16_t keycode, bool shifted, keyrecord_t *record) {
    /*
     * These overrides intentionally bypass "plain Shift" output for selected
     * Finnish-layout symbols. Any change here must be mirrored in
     * autoshift_release_user() so unregister_code16() and repeat-key state stay
     * consistent.
     */
    fleetsing_autoshift_resolution_haptic(shifted, record);

    switch (keycode) {
        case S(FI_4):
        case A(FI_2):
        case FI_DOT:
        case FI_COMM:
        case FI_PLUS:
        case FI_MINS:
        case S(FI_7):
        case S(FI_QUOT):
        case S(FI_DOT):
        case A(FI_DIAE):
        case S(FI_8):
        case A(FI_8):
        case S(FI_9):
        case A(FI_9):
        case FI_QUOT:
        case S(FI_6):
            register_code16(fleetsing_autoshift_output_keycode(keycode, shifted));
            break;
        default:
            if (shifted) {
                add_weak_mods(MOD_BIT(KC_LSFT));
            }
            register_code16((IS_RETRO(keycode)) ? keycode & 0xFF : keycode);
    }
}

void autoshift_release_user(uint16_t keycode, bool shifted, keyrecord_t *record) {
    switch (keycode) {
        case S(FI_4):
        case A(FI_2):
        case FI_DOT:
        case FI_COMM:
        case FI_PLUS:
        case FI_MINS:
        case S(FI_7):
        case S(FI_QUOT):
        case S(FI_DOT):
        case A(FI_DIAE):
        case S(FI_8):
        case A(FI_8):
        case S(FI_9):
        case A(FI_9):
        case FI_QUOT:
        case S(FI_6): {
            uint16_t output_keycode = fleetsing_autoshift_output_keycode(keycode, shifted);

            unregister_code16(output_keycode);
            if (shifted) {
                set_last_keycode(output_keycode);
            }
            break;
        }
        default:
            unregister_code16((IS_RETRO(keycode)) ? keycode & 0xFF : keycode);
    }
}

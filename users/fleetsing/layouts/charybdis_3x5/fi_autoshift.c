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
            register_code16((IS_RETRO(keycode)) ? keycode & 0xFF : keycode);
    }
}

void autoshift_release_user(uint16_t keycode, bool shifted, keyrecord_t *record) {
    switch (keycode) {
        case S(FI_4):
            unregister_code16((!shifted) ? S(FI_4) : A(FI_4));
            if (shifted) {
                set_last_keycode(A(FI_4));
            }
            break;
        case A(FI_2):
            unregister_code16((!shifted) ? A(FI_2) : S(FI_DIAE));
            if (shifted) {
                set_last_keycode(S(FI_DIAE));
            }
            break;
        case FI_DOT:
            unregister_code16((!shifted) ? FI_DOT : S(FI_PLUS));
            if (shifted) {
                set_last_keycode(S(FI_PLUS));
            }
            break;
        case FI_COMM:
            unregister_code16((!shifted) ? FI_COMM : S(FI_1));
            if (shifted) {
                set_last_keycode(S(FI_1));
            }
            break;
        case FI_PLUS:
            unregister_code16((!shifted) ? FI_PLUS : S(FI_0));
            if (shifted) {
                set_last_keycode(S(FI_0));
            }
            break;
        case FI_MINS:
            unregister_code16((!shifted) ? FI_MINS : S(FI_MINS));
            if (shifted) {
                set_last_keycode(S(FI_MINS));
            }
            break;
        case S(FI_7):
            unregister_code16((!shifted) ? S(FI_7) : S(A(FI_7)));
            if (shifted) {
                set_last_keycode(S(A(FI_7)));
            }
            break;
        case S(FI_QUOT):
            unregister_code16((!shifted) ? S(FI_QUOT) : S(FI_3));
            if (shifted) {
                set_last_keycode(S(FI_3));
            }
            break;
        case S(FI_DOT):
            unregister_code16((!shifted) ? S(FI_DOT) : S(FI_COMM));
            if (shifted) {
                set_last_keycode(S(FI_COMM));
            }
            break;
        case A(FI_DIAE):
            unregister_code16((!shifted) ? A(FI_DIAE) : A(FI_7));
            if (shifted) {
                set_last_keycode(A(FI_7));
            }
            break;
        case S(FI_8):
            unregister_code16((!shifted) ? S(FI_8) : S(A(FI_8)));
            if (shifted) {
                set_last_keycode(S(A(FI_8)));
            }
            break;
        case A(FI_8):
            unregister_code16((!shifted) ? A(FI_8) : FI_SECT);
            if (shifted) {
                set_last_keycode(FI_SECT);
            }
            break;
        case S(FI_9):
            unregister_code16((!shifted) ? S(FI_9) : S(A(FI_9)));
            if (shifted) {
                set_last_keycode(S(A(FI_9)));
            }
            break;
        case A(FI_9):
            unregister_code16((!shifted) ? A(FI_9) : S(FI_SECT));
            if (shifted) {
                set_last_keycode(S(FI_SECT));
            }
            break;
        case FI_QUOT:
            unregister_code16((!shifted) ? FI_QUOT : S(FI_2));
            if (shifted) {
                set_last_keycode(S(FI_2));
            }
            break;
        case S(FI_6):
            unregister_code16((!shifted) ? S(FI_6) : S(FI_5));
            if (shifted) {
                set_last_keycode(S(FI_5));
            }
            break;
        default:
            unregister_code16((IS_RETRO(keycode)) ? keycode & 0xFF : keycode);
    }
}

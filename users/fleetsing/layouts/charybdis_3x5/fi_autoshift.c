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
 * Keep the custom Auto Shift key lists in one place so the QMK hook that marks
 * them as custom and the press/release handlers cannot drift apart.
 */
#define FLEETSING_CUSTOM_AUTOSHIFT_SYMBOL_CASES(X) \
    X(S(FI_4))                                     \
    X(A(FI_2))                                     \
    X(FI_DOT)                                      \
    X(FI_COMM)                                     \
    X(FI_PLUS)                                     \
    X(FI_MINS)                                     \
    X(S(FI_7))                                     \
    X(S(FI_QUOT))                                  \
    X(S(FI_DOT))                                   \
    X(A(FI_DIAE))                                  \
    X(S(FI_8))                                     \
    X(A(FI_8))                                     \
    X(S(FI_9))                                     \
    X(A(FI_9))                                     \
    X(FI_QUOT)                                     \
    X(S(FI_6))

#define FLEETSING_CUSTOM_RETRO_AUTOSHIFT_POSITION_CASES(X) \
    X(_L25)                                                \
    X(_L24)                                                \
    X(_L23)                                                \
    X(_L22)                                                \
    X(_L32)                                                \
    X(_L33)                                                \
    X(_R35)                                                \
    X(_R33)                                                \
    X(_R32)                                                \
    X(_R22)                                                \
    X(_R23)                                                \
    X(_R24)                                                \
    X(_R25)

static bool fleetsing_is_custom_autoshift_symbol(uint16_t keycode) {
    switch (keycode) {
#define FLEETSING_CASE(kc) case kc:
        FLEETSING_CUSTOM_AUTOSHIFT_SYMBOL_CASES(FLEETSING_CASE)
            return true;
#undef FLEETSING_CASE
        default:
            return false;
    }
}

static bool fleetsing_is_custom_retro_autoshift_position(uint16_t keycode) {
    switch (keycode) {
#define FLEETSING_CASE(kc) case kc:
        FLEETSING_CUSTOM_RETRO_AUTOSHIFT_POSITION_CASES(FLEETSING_CASE)
            return true;
#undef FLEETSING_CASE
        default:
            return false;
    }
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
    (void)record;

    /*
     * The symbol overrides and selected Retro Shift positions share the same
     * custom Auto Shift hook so QMK routes both through the userspace handlers.
     */
    return fleetsing_is_custom_autoshift_symbol(keycode) || fleetsing_is_custom_retro_autoshift_position(keycode);
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
#define FLEETSING_CASE(kc) case kc:
        FLEETSING_CUSTOM_AUTOSHIFT_SYMBOL_CASES(FLEETSING_CASE)
            register_code16(fleetsing_autoshift_output_keycode(keycode, shifted));
            break;
#undef FLEETSING_CASE
        default:
            if (shifted) {
                add_weak_mods(MOD_BIT(KC_LSFT));
            }
            register_code16((IS_RETRO(keycode)) ? keycode & 0xFF : keycode);
    }
}

void autoshift_release_user(uint16_t keycode, bool shifted, keyrecord_t *record) {
    switch (keycode) {
#define FLEETSING_CASE(kc) case kc:
        FLEETSING_CUSTOM_AUTOSHIFT_SYMBOL_CASES(FLEETSING_CASE) {
            uint16_t output_keycode = fleetsing_autoshift_output_keycode(keycode, shifted);

            unregister_code16(output_keycode);
            if (shifted) {
                set_last_keycode(output_keycode);
            }
            break;
        }
#undef FLEETSING_CASE
        default:
            unregister_code16((IS_RETRO(keycode)) ? keycode & 0xFF : keycode);
    }
}

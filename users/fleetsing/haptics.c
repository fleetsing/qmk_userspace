#include "fleetsing.h"

#ifdef HAPTIC_ENABLE
extern haptic_config_t haptic_config;
#endif

#ifdef HAPTIC_DRV2605L
#    include "drv2605l.h"
#endif

static bool fleetsing_haptic_suppressed;

void fleetsing_haptic_suppress(bool suppress) {
    fleetsing_haptic_suppressed = suppress;
}

/*
 * Haptic filtering should operate on the "effective" key when possible, so a
 * tapped mod-tap behaves like its tap key while a held mod-tap still behaves
 * like a modifier/layer key.
 */
static uint16_t fleetsing_haptic_normalize_keycode(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case QK_MODS ... QK_MODS_MAX:
            return QK_MODS_GET_BASIC_KEYCODE(keycode);
        case QK_MOD_TAP ... QK_MOD_TAP_MAX:
            return record->tap.count == 0 ? keycode : QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
        case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
            return record->tap.count == 0 ? keycode : QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
        default:
            return keycode;
    }
}

__attribute__((weak)) bool get_haptic_enabled_key(uint16_t keycode, keyrecord_t *record) {
    if (fleetsing_haptic_suppressed || IS_ENCODEREVENT(record->event)) {
        return false;
    }

    keycode = fleetsing_haptic_normalize_keycode(keycode, record);

    /*
     * The NO_HAPTIC_* defines in the keymap config act as category filters.
     * Keep the cases grouped by category so the config flags remain readable.
     */
    switch (keycode) {
#ifdef NO_HAPTIC_MOD
        case QK_MOD_TAP ... QK_MOD_TAP_MAX:
        case QK_LAYER_TAP_TOGGLE ... QK_LAYER_TAP_TOGGLE_MAX:
        case QK_LAYER_TAP ... QK_LAYER_TAP_MAX:
        case KC_LEFT_CTRL ... KC_RIGHT_GUI:
        case QK_MOMENTARY ... QK_MOMENTARY_MAX:
        case QK_LAYER_MOD ... QK_LAYER_MOD_MAX:
#endif
#ifdef NO_HAPTIC_ALPHA
        case KC_A ... KC_Z:
#endif
#ifdef NO_HAPTIC_PUNCTUATION
        case KC_ENTER:
        case KC_ESCAPE:
        case KC_BACKSPACE:
        case KC_SPACE:
        case KC_MINUS:
        case KC_EQUAL:
        case KC_LEFT_BRACKET:
        case KC_RIGHT_BRACKET:
        case KC_BACKSLASH:
        case KC_NONUS_HASH:
        case KC_SEMICOLON:
        case KC_QUOTE:
        case KC_GRAVE:
        case KC_COMMA:
        case KC_SLASH:
        case KC_DOT:
        case KC_NONUS_BACKSLASH:
#endif
#ifdef NO_HAPTIC_LOCKKEYS
        case KC_CAPS_LOCK:
        case KC_SCROLL_LOCK:
        case KC_NUM_LOCK:
#endif
#ifdef NO_HAPTIC_NAV
        case KC_PRINT_SCREEN:
        case KC_PAUSE:
        case KC_INSERT:
        case KC_DELETE:
        case KC_PAGE_DOWN:
        case KC_PAGE_UP:
        case KC_LEFT:
        case KC_UP:
        case KC_RIGHT:
        case KC_DOWN:
        case KC_END:
        case KC_HOME:
#endif
#ifdef NO_HAPTIC_NUMERIC
        case KC_1 ... KC_0:
#endif
            return false;
    }

    return true;
}

/*
 * Keep semantic event-to-effect mapping centralized so feature modules can ask
 * for "pointer layer entered" or "macro saved" without caring about the
 * driver-specific DRV2605L sequence numbers underneath.
 */
void fleetsing_haptic_play_event(fleetsing_haptic_event_t event) {
#ifdef HAPTIC_ENABLE
    if (!haptic_get_enable()) {
        return;
    }

#    ifdef HAPTIC_DRV2605L
    uint8_t saved_mode = haptic_config.mode;
    uint8_t effect     = DRV2605L_DEFAULT_MODE;

    switch (event) {
        case FLEETSING_HAPTIC_POINTER_LAYER_ON:
            effect = DRV2605L_EFFECT_SHARP_TICK_1_100;
            break;
        case FLEETSING_HAPTIC_POINTER_LAYER_OFF:
            effect = DRV2605L_EFFECT_SHARP_TICK_3_60;
            break;
        case FLEETSING_HAPTIC_OS_MAC:
            effect = DRV2605L_EFFECT_SOFT_BUMP_100;
            break;
        case FLEETSING_HAPTIC_OS_PC:
            effect = DRV2605L_EFFECT_SHORT_DOUBLE_CLICK_MEDIUM_1_100;
            break;
        case FLEETSING_HAPTIC_LAYER_LOCK_ON:
            effect = DRV2605L_EFFECT_MEDIUM_CLICK_1_100;
            break;
        case FLEETSING_HAPTIC_LAYER_LOCK_OFF:
            effect = DRV2605L_EFFECT_SHARP_TICK_2_80;
            break;
        case FLEETSING_HAPTIC_MACRO_RECORD_START:
            effect = DRV2605L_EFFECT_SHORT_DOUBLE_SHARP_TICK_1_100;
            break;
        case FLEETSING_HAPTIC_MACRO_RECORD_STOP:
            effect = DRV2605L_EFFECT_MEDIUM_CLICK_2_80;
            break;
        case FLEETSING_HAPTIC_MACRO_PLAY:
            effect = DRV2605L_EFFECT_SHORT_DOUBLE_CLICK_STRONG_2_80;
            break;
        case FLEETSING_HAPTIC_EEPROM_CLEAR:
            effect = DRV2605L_EFFECT_750_MS_ALERT_100;
            break;
        case FLEETSING_HAPTIC_BOOTLOADER:
            effect = DRV2605L_EFFECT_LONG_DOUBLE_SHARP_CLICK_STRONG_1_100;
            break;
    }

    haptic_config.mode = effect;
    haptic_play();
    haptic_config.mode = saved_mode;
#    else
    (void)event;
    haptic_play();
#    endif
#else
    (void)event;
#endif
}

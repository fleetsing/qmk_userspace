#include "fleetsing.h"

/* The displays are physically mounted upside down relative to QMK defaults. */
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_180;
}

#ifdef OLED_ENABLE
bool oled_task_user(void) {
    /* Keep this screen compact: highest layer plus host LED state only. */
    oled_write_P(PSTR("Layer:\n"), false);

    switch (get_highest_layer(layer_state)) {
        case LAYER_BASE:
            oled_write_P(PSTR("BASE\n"), false);
            break;
        case LAYER_NUMBERS:
            oled_write_P(PSTR("NUMBER\n"), false);
            break;
        case LAYER_NAVIGATION:
            oled_write_P(PSTR("NAVIGATION\n"), false);
            break;
        case LAYER_POINTER:
            oled_write_P(PSTR("POINTER\n"), false);
            break;
        case LAYER_MACRO:
            oled_write_P(PSTR("MACRO\n"), false);
            break;
        case LAYER_MEDIA:
            oled_write_P(PSTR("MEDIA\n"), false);
            break;
        case LAYER_FUNCTION:
            oled_write_P(PSTR("FUNCTION\n"), false);
            break;
        default:
            // Or use the write_ln shortcut over adding '\n' to the end of your string
            oled_write_ln_P(PSTR("Undefined"), false);
    }

    /* Host LED state reflects the OS, not any local userspace state. */
    led_t led_state = host_keyboard_led_state();
    oled_write_P(led_state.num_lock ? PSTR("NUM ") : PSTR("    "), false);
    oled_write_P(led_state.caps_lock ? PSTR("CAP ") : PSTR("    "), false);
    oled_write_P(led_state.scroll_lock ? PSTR("SCR ") : PSTR("    "), false);

    return false;
}
#endif

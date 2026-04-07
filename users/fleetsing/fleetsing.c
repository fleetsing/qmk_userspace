#include "fleetsing.h"

static bool fleetsing_scrolling_enabled = false;

void fleetsing_set_scrolling_enabled(bool enabled) {
    fleetsing_scrolling_enabled = enabled;
}

bool fleetsing_get_scrolling_enabled(void) {
    return fleetsing_scrolling_enabled;
}

bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!fleetsing_autoshift_haptic_process_record(keycode, record)) {
        return false;
    }

    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!fleetsing_pointing_process_record(keycode, record)) {
        return false;
    }

    if (!fleetsing_repeat_process_record(keycode, record)) {
        return false;
    }

    return true;
}

void matrix_scan_user(void) {
    fleetsing_autoshift_haptic_matrix_scan();
}

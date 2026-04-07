#include "fleetsing.h"

static fleetsing_scroll_side_t fleetsing_scroll_side = FLEETSING_SCROLL_SIDE_LEFT;

void fleetsing_set_scroll_side(fleetsing_scroll_side_t side) {
    fleetsing_scroll_side = side;
}

fleetsing_scroll_side_t fleetsing_get_scroll_side(void) {
    return fleetsing_scroll_side;
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

    return true;
}

void matrix_scan_user(void) {
    fleetsing_autoshift_haptic_matrix_scan();
}

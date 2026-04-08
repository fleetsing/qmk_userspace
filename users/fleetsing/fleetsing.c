#include "fleetsing.h"

/*
 * This file is the generic userspace hook dispatcher.
 *
 * Keep keyboard- or feature-specific logic in dedicated modules and only route
 * the QMK hooks here. That makes it easier to see which hook owns which
 * behavior and avoids accidental hook duplication across files.
 */
static fleetsing_scroll_side_t fleetsing_scroll_side = FLEETSING_SCROLL_SIDE_LEFT;

void fleetsing_set_scroll_side(fleetsing_scroll_side_t side) {
    fleetsing_scroll_side = side;
}

fleetsing_scroll_side_t fleetsing_get_scroll_side(void) {
    return fleetsing_scroll_side;
}

bool pre_process_record_user(uint16_t keycode, keyrecord_t *record) {
    /*
     * Treat any physical key event as OLED activity.
     *
     * This userspace-owned timer is more predictable than relying on generic
     * core activity timestamps when split pointing is also active.
     */
    fleetsing_display_note_activity();

    /* Pre-process runs before QMK's normal key handling and is used here for haptic timing. */
    if (!fleetsing_autoshift_haptic_process_record(keycode, record)) {
        return false;
    }

    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    /* Main per-key hook for userspace-owned custom keycodes. */
    if (!fleetsing_pointing_process_record(keycode, record)) {
        return false;
    }

    return true;
}

void matrix_scan_user(void) {
    /* Poll for delayed Auto Shift haptic feedback without blocking key processing. */
    fleetsing_autoshift_haptic_matrix_scan();
}

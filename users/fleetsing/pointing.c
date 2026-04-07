#include "fleetsing.h"

#ifdef POINTING_DEVICE_ENABLE
#    define SCROLL_DIVISOR_H 50.0
#    define SCROLL_DIVISOR_V 50.0

float scroll_accumulated_h = 0;
float scroll_accumulated_v = 0;
#endif

bool fleetsing_pointing_process_record(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case SET_MS_L:
            if (record->event.pressed) {
                fleetsing_set_scrolling_enabled(true);
            }
            return false;
        case SET_MS_R:
            if (record->event.pressed) {
                fleetsing_set_scrolling_enabled(false);
            }
            return false;
        default:
            return true;
    }
}

#ifdef POINTING_DEVICE_COMBINED
report_mouse_t pointing_device_task_combined_user(report_mouse_t left_report, report_mouse_t right_report) {
    if (fleetsing_get_scrolling_enabled()) {
        scroll_accumulated_h += (float)right_report.x / SCROLL_DIVISOR_H;
        scroll_accumulated_v += (float)right_report.y / SCROLL_DIVISOR_V;

        right_report.h = (int8_t)scroll_accumulated_h;
        right_report.v = -(int8_t)scroll_accumulated_v;

        scroll_accumulated_h -= (int8_t)scroll_accumulated_h;
        scroll_accumulated_v -= (int8_t)scroll_accumulated_v;

        right_report.x = 0;
        right_report.y = 0;
    } else {
        scroll_accumulated_h += (float)left_report.x / SCROLL_DIVISOR_H;
        scroll_accumulated_v += (float)left_report.y / SCROLL_DIVISOR_V;

        left_report.h = (int8_t)scroll_accumulated_h;
        left_report.v = -(int8_t)scroll_accumulated_v;

        scroll_accumulated_h -= (int8_t)scroll_accumulated_h;
        scroll_accumulated_v -= (int8_t)scroll_accumulated_v;

        left_report.x = 0;
        left_report.y = 0;
    }

    return pointing_device_combine_reports(left_report, right_report);
}
#endif

#ifdef POINTING_DEVICE_ENABLE
layer_state_t layer_state_set_user(layer_state_t state) {
    charybdis_set_pointer_sniping_enabled(layer_state_cmp(state, FLEETSING_AUTO_SNIPING_LAYER));
    return state;
}
#endif

void keyboard_post_init_user(void) {
#ifdef POINTING_DEVICE_COMBINED
    if (!is_keyboard_master()) {
        pointing_device_set_cpi_on_side(is_keyboard_left(), PMW33XX_CPI - 1000);
    }
#endif
}

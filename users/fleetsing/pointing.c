#include "fleetsing.h"

#ifdef POINTING_DEVICE_ENABLE
bool set_scrolling = false;
// Modify these values to adjust the scrolling speed
#define SCROLL_DIVISOR_H 50.0
#define SCROLL_DIVISOR_V 50.0

// Variables to store accumulated scroll values
float scroll_accumulated_h = 0;
float scroll_accumulated_v = 0;
#endif

#ifdef POINTING_DEVICE_COMBINED
report_mouse_t pointing_device_task_combined_user(report_mouse_t left_report, report_mouse_t right_report) {
    if (fleetsing_get_scrolling_enabled()) {
        // Calculate and accumulate scroll values based on mouse movement and divisors
        scroll_accumulated_h += (float)right_report.x / SCROLL_DIVISOR_H;
        scroll_accumulated_v += (float)right_report.y / SCROLL_DIVISOR_V;

        // Assign integer parts of accumulated scroll values to the mouse report
        right_report.h = (int8_t)scroll_accumulated_h;
        right_report.v = -(int8_t)scroll_accumulated_v;

        // Update accumulated scroll values by subtracting the integer parts
        scroll_accumulated_h -= (int8_t)scroll_accumulated_h;
        scroll_accumulated_v -= (int8_t)scroll_accumulated_v;

        // Clear the X and Y values of the mouse report
        right_report.x = 0;
        right_report.y = 0;
    } else {
        // Calculate and accumulate scroll values based on mouse movement and divisors
        scroll_accumulated_h += (float)left_report.x / SCROLL_DIVISOR_H;
        scroll_accumulated_v += (float)left_report.y / SCROLL_DIVISOR_V;

        // Assign integer parts of accumulated scroll values to the mouse report
        left_report.h = (int8_t)scroll_accumulated_h;
        left_report.v = -(int8_t)scroll_accumulated_v;

        // Update accumulated scroll values by subtracting the integer parts
        scroll_accumulated_h -= (int8_t)scroll_accumulated_h;
        scroll_accumulated_v -= (int8_t)scroll_accumulated_v;

        left_report.x = 0;
        left_report.y = 0;
    }

    // Clear the X and Y values of the mouse report

    return pointing_device_combine_reports(left_report, right_report);
}
#endif

void keyboard_post_init_user(void) {
    // Customise these values to desired behaviour
    // debug_enable=true;
    // debug_matrix=true;
    // debug_keyboard=true;
    // debug_mouse=true;
    #ifdef POINTING_DEVICE_COMBINED
      // Hack to fix slave side being a lower sensitivity
    if (!is_keyboard_master()) {
        pointing_device_set_cpi_on_side(is_keyboard_left(), PMW33XX_CPI - 1000);
    }
//        pointing_device_set_cpi_on_side(false, PMW33XX_CPI);
//        pointing_device_set_cpi_on_side(true, PMW33XX_CPI);
    #endif
}
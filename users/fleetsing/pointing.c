#include "fleetsing.h"

#ifdef POINTING_DEVICE_ENABLE
/*
 * Higher divisors make scrolling slower. These must stay positive and should
 * usually be tuned together so diagonal scroll gestures keep a predictable
 * ratio. They only affect the userspace scroll transform, not sensor DPI.
 */
#    define SCROLL_DIVISOR_H 133.0
#    define SCROLL_DIVISOR_V 133.0

static float scroll_accumulated_h = 0;
static float scroll_accumulated_v = 0;
#endif

#ifdef POINTING_DEVICE_COMBINED
static void fleetsing_reset_scroll_accumulators(void) {
    scroll_accumulated_h = 0;
    scroll_accumulated_v = 0;
}

/*
 * Treat any non-zero pointing-device report as real user activity.
 *
 * This is narrower than QMK's generic pointing activity path: it only resets
 * the OLED idle timer when one of the trackballs actually produces movement,
 * scroll, or button data that survives into userspace.
 */
static bool fleetsing_mouse_report_has_activity(report_mouse_t report) {
    return report.x != 0 || report.y != 0 || report.h != 0 || report.v != 0 || report.buttons != 0;
}
#endif

bool fleetsing_pointing_process_record(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case SET_MS_L:
            if (record->event.pressed) {
                if (fleetsing_get_scroll_side() != FLEETSING_SCROLL_SIDE_LEFT) {
                    fleetsing_haptic_play_event(FLEETSING_HAPTIC_SCROLL_SIDE);
                }
                /*
                 * Reset any fractional carry so leftover motion from the
                 * previous side does not leak into the newly selected side.
                 */
                fleetsing_set_scroll_side(FLEETSING_SCROLL_SIDE_LEFT);
#ifdef POINTING_DEVICE_COMBINED
                fleetsing_reset_scroll_accumulators();
#endif
            }
            return false;
        case SET_MS_R:
            if (record->event.pressed) {
                if (fleetsing_get_scroll_side() != FLEETSING_SCROLL_SIDE_RIGHT) {
                    fleetsing_haptic_play_event(FLEETSING_HAPTIC_SCROLL_SIDE);
                }
                fleetsing_set_scroll_side(FLEETSING_SCROLL_SIDE_RIGHT);
#ifdef POINTING_DEVICE_COMBINED
                fleetsing_reset_scroll_accumulators();
#endif
            }
            return false;
        default:
            return true;
    }
}

#ifdef POINTING_DEVICE_COMBINED
report_mouse_t pointing_device_task_combined_user(report_mouse_t left_report, report_mouse_t right_report) {
    /*
     * Only the selected side is transformed into scroll. The other side keeps
     * acting as a normal pointer source, and DPI remains managed by Charybdis.
     */
    if (fleetsing_mouse_report_has_activity(left_report) || fleetsing_mouse_report_has_activity(right_report)) {
        fleetsing_display_note_activity();
    }

    if (fleetsing_get_scroll_side() == FLEETSING_SCROLL_SIDE_RIGHT) {
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
    bool sniping_was_enabled = charybdis_get_pointer_sniping_enabled();
    bool sniping_enabled     = layer_state_cmp(state, FLEETSING_AUTO_SNIPING_LAYER);

    /*
     * Pointer-layer activation drives sniping mode, but the actual CPI change
     * is applied in the firmware's Charybdis layer.
     */
    charybdis_set_pointer_sniping_enabled(sniping_enabled);
    if (sniping_was_enabled != sniping_enabled) {
        fleetsing_haptic_play_event(sniping_enabled ? FLEETSING_HAPTIC_POINTER_LAYER_ON : FLEETSING_HAPTIC_POINTER_LAYER_OFF);
    }

    return state;
}
#endif

void keyboard_post_init_user(void) {
    /* Start the OLED idle timer in the "recently active" state after boot. */
    fleetsing_display_note_activity();

#ifdef HAPTIC_ENABLE
    if (haptic_get_mode() != DRV2605L_DEFAULT_MODE) {
        haptic_set_mode(DRV2605L_DEFAULT_MODE);
    }
#endif
}

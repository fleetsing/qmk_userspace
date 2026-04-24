#include "fleetsing.h"

#ifdef POINTING_DEVICE_ENABLE
/*
 * Treat any non-zero pointing-device report as real user activity.
 *
 * This is narrower than QMK's generic pointing activity path: it only resets
 * the OLED idle timer when the remaining trackball actually produces movement,
 * scroll, or button data that survives into userspace.
 */
static bool fleetsing_mouse_report_has_activity(report_mouse_t report) {
    return report.x != 0 || report.y != 0 || report.h != 0 || report.v != 0 || report.buttons != 0;
}
#endif

bool fleetsing_pointing_process_record(uint16_t keycode, keyrecord_t *record) {
    (void)keycode;
    (void)record;
    return true;
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (fleetsing_mouse_report_has_activity(mouse_report)) {
        fleetsing_display_note_activity();
    }

    return mouse_report;
}

layer_state_t fleetsing_pointing_layer_state_set(layer_state_t state) {
#ifdef POINTING_DEVICE_ENABLE
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
#endif

    return state;
}

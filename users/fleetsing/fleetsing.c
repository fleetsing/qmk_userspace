#include "fleetsing.h"

/*
 * This file is the generic userspace hook dispatcher.
 *
 * Keep keyboard- or feature-specific logic in dedicated modules and only route
 * the QMK hooks here. That makes it easier to see which hook owns which
 * behavior and avoids accidental hook duplication across files.
 */
static fleetsing_scroll_side_t fleetsing_scroll_side = FLEETSING_SCROLL_SIDE_LEFT;
static layer_state_t           fleetsing_locked_layers_before;
static bool                    fleetsing_layer_lock_pending;

static layer_state_t fleetsing_locked_layers_mask(void) {
    layer_state_t locked_layers = 0;

    if (is_layer_locked(LAYER_NUMBERS)) {
        locked_layers |= (layer_state_t)1 << LAYER_NUMBERS;
    }
    if (is_layer_locked(LAYER_NAVIGATION)) {
        locked_layers |= (layer_state_t)1 << LAYER_NAVIGATION;
    }
    if (is_layer_locked(LAYER_FUNCTION)) {
        locked_layers |= (layer_state_t)1 << LAYER_FUNCTION;
    }
    if (is_layer_locked(LAYER_MEDIA)) {
        locked_layers |= (layer_state_t)1 << LAYER_MEDIA;
    }
    if (is_layer_locked(LAYER_POINTER)) {
        locked_layers |= (layer_state_t)1 << LAYER_POINTER;
    }
    if (is_layer_locked(LAYER_MACRO)) {
        locked_layers |= (layer_state_t)1 << LAYER_MACRO;
    }

    return locked_layers;
}

void fleetsing_set_scroll_side(fleetsing_scroll_side_t side) {
    fleetsing_scroll_side = side;
}

fleetsing_scroll_side_t fleetsing_get_scroll_side(void) {
    return fleetsing_scroll_side;
}

/*
 * OS mode is derived from QMK's persisted Ctrl/GUI swap flags.
 *
 * This keeps the modifier remap and the visible userspace "OS mode" indicator
 * backed by the same EEPROM state instead of maintaining a second source of
 * truth. Userspace treats any active swap as PC mode so the OLED does not have
 * to expose an awkward half-swapped intermediate state.
 */
fleetsing_os_mode_t fleetsing_get_os_mode(void) {
    return (keymap_config.swap_lctl_lgui || keymap_config.swap_rctl_rgui) ? FLEETSING_OS_PC : FLEETSING_OS_MAC;
}

void fleetsing_set_os_mode(fleetsing_os_mode_t mode) {
    bool pc_mode = mode == FLEETSING_OS_PC;

    if (keymap_config.swap_lctl_lgui == pc_mode && keymap_config.swap_rctl_rgui == pc_mode) {
        return;
    }

    keymap_config.swap_lctl_lgui = pc_mode;
    keymap_config.swap_rctl_rgui = pc_mode;
    eeconfig_update_keymap(&keymap_config);
    fleetsing_haptic_play_event(pc_mode ? FLEETSING_HAPTIC_OS_PC : FLEETSING_HAPTIC_OS_MAC);
}

const char *fleetsing_get_os_mode_name(void) {
    return fleetsing_get_os_mode() == FLEETSING_OS_PC ? "PC" : "MAC";
}

/*
 * Layout-specific modules use this helper when the same intended symbol needs
 * a different raw keycode between macOS and PC mode.
 */
uint16_t fleetsing_os_keycode(uint16_t mac_keycode, uint16_t pc_keycode) {
    return fleetsing_get_os_mode() == FLEETSING_OS_PC ? pc_keycode : mac_keycode;
}

/*
 * Keep OS-mode selectors in one shared hook so persistence stays in one file.
 *
 * Symbol routing is intentionally not handled here anymore. The affected combo
 * outputs keep their original raw keycodes, and the layout-specific Auto Shift
 * helpers branch by OS mode where needed so Retro Shift keeps behaving like it
 * did before OS-mode support was added.
 */
bool fleetsing_os_process_record(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case OS_MAC:
            if (record->event.pressed) {
                fleetsing_set_os_mode(FLEETSING_OS_MAC);
            }
            return false;
        case OS_PC:
            if (record->event.pressed) {
                fleetsing_set_os_mode(FLEETSING_OS_PC);
            }
            return false;
        default:
            break;
    }

    return true;
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
    if (record->event.pressed) {
        switch (keycode) {
            case QK_LAYER_LOCK:
                fleetsing_locked_layers_before = fleetsing_locked_layers_mask();
                fleetsing_layer_lock_pending   = true;
                break;
            case EE_CLR:
                fleetsing_haptic_play_event(FLEETSING_HAPTIC_EEPROM_CLEAR);
                break;
            case QK_BOOT:
                fleetsing_haptic_play_event(FLEETSING_HAPTIC_BOOTLOADER);
                break;
            default:
                break;
        }
    }

    /* Main per-key hook for userspace-owned custom keycodes. */
    if (!fleetsing_pointing_process_record(keycode, record)) {
        return false;
    }

    if (!fleetsing_os_process_record(keycode, record)) {
        return false;
    }

    return true;
}

void post_process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed || keycode != QK_LAYER_LOCK || !fleetsing_layer_lock_pending) {
        return;
    }

    layer_state_t locked_layers_after = fleetsing_locked_layers_mask();
    layer_state_t newly_locked_layers = locked_layers_after & ~fleetsing_locked_layers_before;

    if (locked_layers_after != fleetsing_locked_layers_before) {
        fleetsing_haptic_play_event(newly_locked_layers ? FLEETSING_HAPTIC_LAYER_LOCK_ON : FLEETSING_HAPTIC_LAYER_LOCK_OFF);
    }

    fleetsing_layer_lock_pending = false;
}

void matrix_scan_user(void) {
    /* Poll for delayed Auto Shift haptic feedback without blocking key processing. */
    fleetsing_autoshift_haptic_matrix_scan();
}

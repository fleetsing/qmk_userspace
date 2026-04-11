#include "fleetsing.h"
#include <string.h>

#ifdef SPLIT_TRANSACTION_IDS_USER
#    include "transactions.h"
#endif

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
static bool                    fleetsing_numword_active;
static uint16_t                fleetsing_numword_timer;
typedef struct {
    bool     active;
    bool     fired;
    uint16_t start_time;
} fleetsing_hold_action_t;

static fleetsing_hold_action_t fleetsing_boot_hold;

#ifndef FLEETSING_BOOT_HOLD_TERM
#    define FLEETSING_BOOT_HOLD_TERM 600
#endif

typedef struct {
    bool     active;
    uint16_t remaining_ms;
} fleetsing_numword_sync_t;

#ifdef SPLIT_TRANSACTION_IDS_USER
static fleetsing_numword_sync_t fleetsing_numword_remote_state;
#endif

static void fleetsing_numword_reset_timer(void) {
#if FLEETSING_NUMWORD_IDLE_TIMEOUT > 0
    fleetsing_numword_timer = timer_read();
#endif
}

static void fleetsing_numword_on(void) {
    if (fleetsing_numword_active) {
        fleetsing_numword_reset_timer();
        return;
    }

    fleetsing_numword_active = true;
    layer_on(LAYER_NUMWORD);
    fleetsing_numword_reset_timer();
}

static void fleetsing_numword_off(void) {
    if (!fleetsing_numword_active) {
        return;
    }

    fleetsing_numword_active = false;
    layer_off(LAYER_NUMWORD);
}

/*
 * NumWord stays active for digits, editing, cursor movement, repeat, and the
 * raw symbol keycodes produced by the positional combos and Auto Shift
 * overrides.
 */
static bool fleetsing_numword_continue(uint16_t keycode) {
    switch (keycode) {
        case FI_1:
        case FI_2:
        case FI_3:
        case FI_4:
        case FI_5:
        case FI_6:
        case FI_7:
        case FI_8:
        case FI_9:
        case FI_0:
        case FI_DOT:
        case FI_COMM:
        case FI_COLN:
        case FI_SCLN:
        case FI_PLUS:
        case FI_MINS:
        case FI_UNDS:
        case FI_SLSH:
        case FI_ASTR:
        case FI_PERC:
        case FI_AMPR:
        case FI_LPRN:
        case FI_RPRN:
        case FI_EQL:
        case FI_QUOT:
        case FI_AT:
        case FI_BSLS:
        case FI_LABK:
        case FI_RABK:
        case FI_LBRC:
        case FI_RBRC:
        case FI_LCBR:
        case FI_RCBR:
        case KC_BSPC:
        case KC_DEL:
        case KC_LEFT:
        case KC_RGHT:
        case KC_UP:
        case KC_DOWN:
        case KC_HOME:
        case KC_END:
        case KC_PGUP:
        case KC_PGDN:
        case KC_INS:
        case QK_REP:
        case QK_AREP:
            return true;

        default:
            return fleetsing_is_symbol_combo_keycode(keycode);
    }
}

static bool fleetsing_numword_should_ignore(uint16_t keycode, keyrecord_t *record) {
    (void)record;

    switch (keycode) {
        case QK_MOMENTARY ... QK_MOMENTARY_MAX:
        case QK_TO ... QK_TO_MAX:
        case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX:
        case QK_LAYER_TAP_TOGGLE ... QK_LAYER_TAP_TOGGLE_MAX:
        case QK_ONE_SHOT_LAYER ... QK_ONE_SHOT_LAYER_MAX:
#ifdef TRI_LAYER_ENABLE
        case QK_TRI_LAYER_LOWER ... QK_TRI_LAYER_UPPER:
#endif
#ifdef LAYER_LOCK_ENABLE
        case QK_LAYER_LOCK:
#endif
        case OSM(MOD_LSFT):
        case OSM(MOD_RALT):
        case KC_LSFT:
        case KC_RSFT:
        case KC_LCTL:
        case KC_RCTL:
        case KC_LALT:
        case KC_RALT:
        case KC_LGUI:
        case KC_RGUI:
            return true;
        default:
            return false;
    }
}

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
    if (is_layer_locked(LAYER_SYMBOLS)) {
        locked_layers |= (layer_state_t)1 << LAYER_SYMBOLS;
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

/*
 * Keep maintenance actions one-handed and mirrored, but require explicit hold
 * intent so transient pointer-layer entry cannot trigger them by accident.
 */
static void fleetsing_trigger_safe_boot(void) {
    clear_keyboard();
    fleetsing_haptic_play_event(FLEETSING_HAPTIC_BOOTLOADER);
    reset_keyboard();
}

static bool fleetsing_maintenance_process_record(uint16_t keycode, keyrecord_t *record) {
    fleetsing_hold_action_t *state = NULL;

    switch (keycode) {
        case BOOT_SAFE:
            state = &fleetsing_boot_hold;
            break;
        default:
            return true;
    }

    if (record->event.pressed) {
        state->active     = true;
        state->fired      = false;
        state->start_time = timer_read();
    } else {
        state->active = false;
        state->fired  = false;
    }

    return false;
}

static void fleetsing_maintenance_task(void) {
    if (fleetsing_boot_hold.active && !fleetsing_boot_hold.fired && timer_elapsed(fleetsing_boot_hold.start_time) >= FLEETSING_BOOT_HOLD_TERM) {
        fleetsing_boot_hold.fired  = true;
        fleetsing_boot_hold.active = false;
        fleetsing_trigger_safe_boot();
    }
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

static bool fleetsing_caps_word_process_record(uint16_t keycode, keyrecord_t *record) {
    if (keycode == CAPSWORD) {
        if (record->event.pressed) {
            caps_word_on();
        }
        return false;
    }

    return true;
}

bool fleetsing_symbol_process_record(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) {
        return true;
    }

    /*
     * Finnish-on-macOS symbol quirks live here.
     *
     * The active OS mode still assumes a Finnish keyboard layout, but macOS and
     * PC layouts disagree on several "coding punctuation" chords. In particular:
     * - macOS uses Option+7 for pipe and Option+Shift+7 for backslash
     * - macOS angle brackets come from the section/grave key instead of FI_LABK
     * - brackets, braces, at-sign, dollar, and tilde also use different chords
     *
     * Keep these translations centralized so layer keys and combos can refer to
     * logical symbol keycodes instead of encoding platform-specific raw chords.
     */
    switch (keycode) {
        case SYM_AT:
            tap_code16(fleetsing_os_keycode(A(FI_2), FI_AT));
            return false;
        case SYM_DLR:
            tap_code16(fleetsing_os_keycode(A(FI_4), FI_DLR));
            return false;
        case SYM_LBRC:
            tap_code16(fleetsing_os_keycode(A(FI_8), FI_LBRC));
            return false;
        case SYM_RBRC:
            tap_code16(fleetsing_os_keycode(A(FI_9), FI_RBRC));
            return false;
        case SYM_LCBR:
            tap_code16(fleetsing_os_keycode(S(A(FI_8)), FI_LCBR));
            return false;
        case SYM_RCBR:
            tap_code16(fleetsing_os_keycode(S(A(FI_9)), FI_RCBR));
            return false;
        case SYM_LABK:
            tap_code16(fleetsing_os_keycode(FI_SECT, FI_LABK));
            return false;
        case SYM_RABK:
            tap_code16(fleetsing_os_keycode(S(FI_SECT), FI_RABK));
            return false;
        case SYM_BSLS:
            tap_code16(fleetsing_os_keycode(S(A(FI_7)), FI_BSLS));
            return false;
        case SYM_PIPE:
            tap_code16(fleetsing_os_keycode(A(FI_7), FI_PIPE));
            return false;
        case SYM_TILD:
            tap_code16(fleetsing_os_keycode(A(FI_DIAE), FI_TILD));
            return false;
        default:
            return true;
    }
}

bool fleetsing_numword_process_record(uint16_t keycode, keyrecord_t *record) {
    if (keycode == NUMWORD) {
        if (record->event.pressed) {
            if (fleetsing_numword_active) {
                fleetsing_numword_off();
            } else {
                fleetsing_numword_on();
            }
        }
        return false;
    }

    if (keycode == NUMLOCK) {
        if (record->event.pressed) {
            fleetsing_numword_off();
            layer_lock_on(LAYER_NUMBERS);
        }
        return false;
    }

    if (!fleetsing_numword_active) {
        return true;
    }

    if (!record->event.pressed || fleetsing_numword_should_ignore(keycode, record)) {
        return true;
    }

#ifndef NO_ACTION_TAPPING
    if (keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX && record->tap.count != 0) {
        keycode = QK_MOD_TAP_GET_TAP_KEYCODE(keycode);
    } else if (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX && record->tap.count != 0) {
        keycode = QK_LAYER_TAP_GET_TAP_KEYCODE(keycode);
    }
#endif

    if (fleetsing_numword_continue(keycode)) {
        fleetsing_numword_reset_timer();
    } else {
        fleetsing_numword_off();
    }

    return true;
}

bool fleetsing_numword_is_active(void) {
    return fleetsing_numword_active;
}

uint16_t fleetsing_numword_idle_remaining(void) {
#if FLEETSING_NUMWORD_IDLE_TIMEOUT > 0
    if (!fleetsing_numword_active) {
        return 0;
    }

    uint16_t elapsed = timer_elapsed(fleetsing_numword_timer);
    return elapsed >= FLEETSING_NUMWORD_IDLE_TIMEOUT ? 0 : FLEETSING_NUMWORD_IDLE_TIMEOUT - elapsed;
#else
    return fleetsing_numword_active ? UINT16_MAX : 0;
#endif
}

bool fleetsing_numword_display_is_active(void) {
#ifdef SPLIT_TRANSACTION_IDS_USER
    if (!is_keyboard_master()) {
        return fleetsing_numword_remote_state.active;
    }
#endif
    return fleetsing_numword_active;
}

uint16_t fleetsing_numword_display_remaining(void) {
#ifdef SPLIT_TRANSACTION_IDS_USER
    if (!is_keyboard_master()) {
        return fleetsing_numword_remote_state.remaining_ms;
    }
#endif
    return fleetsing_numword_idle_remaining();
}

#ifdef SPLIT_TRANSACTION_IDS_USER
static void fleetsing_numword_sync_handler(uint8_t initiator2target_buffer_size, const void *initiator2target_buffer, uint8_t target2initiator_buffer_size, void *target2initiator_buffer) {
    (void)target2initiator_buffer_size;
    (void)target2initiator_buffer;

    if (initiator2target_buffer_size == sizeof(fleetsing_numword_remote_state)) {
        memcpy(&fleetsing_numword_remote_state, initiator2target_buffer, sizeof(fleetsing_numword_remote_state));
    }
}

static void fleetsing_numword_sync_task(void) {
    if (!is_keyboard_master()) {
        return;
    }

    static fleetsing_numword_sync_t last_sent_state = {0};
    static uint32_t                 last_sync       = 0;
    fleetsing_numword_sync_t        current_state   = {
        .active       = fleetsing_numword_active,
        .remaining_ms = fleetsing_numword_idle_remaining(),
    };
    bool needs_sync = memcmp(&current_state, &last_sent_state, sizeof(current_state)) != 0;

    if (!needs_sync && timer_elapsed32(last_sync) <= 500) {
        return;
    }

    if (transaction_rpc_send(RPC_ID_USER_NUMWORD_SYNC, sizeof(current_state), &current_state)) {
        memcpy(&last_sent_state, &current_state, sizeof(current_state));
        last_sync = timer_read32();
    }
}
#endif

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
            default:
                break;
        }
    }

    /* Main per-key hook for userspace-owned custom keycodes. */
    if (!fleetsing_maintenance_process_record(keycode, record)) {
        return false;
    }

    if (!fleetsing_pointing_process_record(keycode, record)) {
        return false;
    }

    if (!fleetsing_caps_word_process_record(keycode, record)) {
        return false;
    }

    if (!fleetsing_numword_process_record(keycode, record)) {
        return false;
    }

    if (!fleetsing_symbol_process_record(keycode, record)) {
        return false;
    }

    if (!fleetsing_os_process_record(keycode, record)) {
        return false;
    }

    return true;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    /*
     * Holding the symmetric main thumbs together enters the media/system layer:
     * Numbers on the left thumb plus Navigation on the right thumb promotes to
     * Media regardless of which thumb is pressed first.
     *
     * Keep this tri-layer rule in the generic userspace hook so it does not
     * depend on whether pointing-device support is compiled in.
     */
    state = update_tri_layer_state(state, LAYER_NUMBERS, LAYER_NAVIGATION, LAYER_MEDIA);
    return fleetsing_pointing_layer_state_set(state);
}

/*
 * Extend Caps Word for Finnish letters and identifier-friendly separators.
 *
 * The core default is US-centric and would treat the raw key positions for
 * Å/Ä/Ö as punctuation. Keep those keys in the same "letter" bucket as A-Z so
 * Caps Word works for Finnish words as well as all-caps identifiers.
 *
 * Continue through the punctuation that often appears inside code-oriented
 * names and paths so sequences such as FOO_BAR, FOO-BAR, FOO/BAR, and
 * CONFIG.H stay in Caps Word without re-triggering it.
 */
bool caps_word_press_user(uint16_t keycode) {
    switch (keycode) {
        case KC_A ... KC_Z:
        case FI_ARNG:
        case FI_ADIA:
        case FI_ODIA:
            add_weak_mods(MOD_BIT(KC_LSFT));
            return true;

        case KC_1 ... KC_0:
        case KC_BSPC:
        case KC_DEL:
        case FI_DOT:
        case FI_MINS:
        case FI_SLSH:
        case FI_COLN:
        case FI_BSLS:
        case FI_UNDS:
        case SYM_BSLS:
            return true;

        default:
            return false;
    }
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
    fleetsing_maintenance_task();
    fleetsing_numword_task();
#ifdef SPLIT_TRANSACTION_IDS_USER
    fleetsing_numword_sync_task();
#endif
    fleetsing_display_sync_task();
    /* Poll for delayed Auto Shift haptic feedback without blocking key processing. */
    fleetsing_autoshift_haptic_matrix_scan();
}

void fleetsing_numword_task(void) {
#if FLEETSING_NUMWORD_IDLE_TIMEOUT > 0
    if (fleetsing_numword_active && timer_elapsed(fleetsing_numword_timer) >= FLEETSING_NUMWORD_IDLE_TIMEOUT) {
        fleetsing_numword_off();
    }
#endif
}

void keyboard_post_init_user(void) {
    /* Start the OLED idle timer in the "recently active" state after boot. */
    fleetsing_display_note_activity();

#ifdef SPLIT_TRANSACTION_IDS_USER
    transaction_register_rpc(RPC_ID_USER_NUMWORD_SYNC, fleetsing_numword_sync_handler);
#endif
    fleetsing_display_post_init();

#ifdef HAPTIC_ENABLE
    if (haptic_get_mode() != DRV2605L_DEFAULT_MODE) {
        haptic_set_mode(DRV2605L_DEFAULT_MODE);
    }
#endif
}

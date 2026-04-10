#include "fleetsing.h"

#include <stdio.h>
#include <string.h>

#ifndef CHARYBDIS_DRAGSCROLL_DPI
#    define CHARYBDIS_DRAGSCROLL_DPI 100
#endif

#ifndef FLEETSING_OLED_IDLE_TIMEOUT
#    define FLEETSING_OLED_IDLE_TIMEOUT 30000
#endif

#ifndef FLEETSING_OLED_OVERLAY_TIMEOUT
#    define FLEETSING_OLED_OVERLAY_TIMEOUT 1500
#endif

#ifndef FLEETSING_OLED_OVERLAY_GROUP_WINDOW
#    define FLEETSING_OLED_OVERLAY_GROUP_WINDOW 250
#endif

#define FLEETSING_OLED_VALUE_SIZE 16
#define FLEETSING_OLED_SNAPSHOT_SIZE 128
#define FLEETSING_OLED_OVERLAY_MAX_ITEMS 3
#define FLEETSING_OLED_MASTER_TOP_PAD 2
#define FLEETSING_OLED_OFFHAND_TOP_PAD 2
#define FLEETSING_OLED_TEMP_TOP_PAD 2

#ifdef DYNAMIC_MACRO_ENABLE
/*
 * The macro layer uses QMK's two built-in dynamic macro slots.
 *
 * The OLED only needs a small summary of what the recorder is doing, so this
 * enum stores display-oriented states rather than the full internal macro data.
 */
typedef enum {
    FLEETSING_MACRO_IDLE = 0,
    FLEETSING_MACRO_REC1,
    FLEETSING_MACRO_REC2,
    FLEETSING_MACRO_PLAY1,
    FLEETSING_MACRO_PLAY2,
    FLEETSING_MACRO_SAVE1,
    FLEETSING_MACRO_SAVE2,
} fleetsing_macro_status_t;

static fleetsing_macro_status_t fleetsing_macro_status       = FLEETSING_MACRO_IDLE;
static uint32_t                 fleetsing_macro_status_timer = 0;

static void fleetsing_refresh_macro_status(void);
#endif

/*
 * OLED glossary:
 * - Layer: highest currently active layer on the master half.
 * - Lock: layers latched with QMK Layer Lock, not merely held.
 * - Ptr: two mode flags plus the selected scroll source, shown as "<SD> <L/R>".
 *        S = sniping, D = drag-scroll, L/R = left/right sensor becomes scroll.
 * - DPI: effective pointer CPI for the current pointer mode.
 * - OS: persisted shortcut/symbol mode. MAC keeps Cmd-native behavior, while
 *       PC swaps Ctrl/GUI so the Command-position keys behave more like Ctrl.
 * - Macro: dynamic macro recorder/playback status for slot 1 or 2.
 * - OSM: one-shot modifiers in Shift, Ctrl, Alt, GUI order.
 *        Lowercase means armed for the next key, uppercase means locked.
 * - Host: host keyboard LED state in Num, Caps, Scroll order.
 * - Alert: an offhand warning-first summary for unusual active conditions.
 * - Overlay: a short-lived bundled confirmation page that can show several
 *            changed states together, such as layer + DPI on pointer-layer entry.
 * - Macro Page: a temporary offhand detail page during macro recording,
 *               playback, or recent save acknowledgement.
 * - NumWord Page: a temporary right-side detail page that shows the synced
 *                 NumWord timeout as both a countdown and a compact progress bar.
 */

/*
 * Keep the on-screen layer names short and stable.
 *
 * The OLED is tall but still narrow enough that verbose labels become hard to
 * scan quickly, so the renderer uses these compact names everywhere.
 */
static const char *fleetsing_layer_name(uint8_t layer) {
    switch (layer) {
        case LAYER_BASE:
            return "BASE";
        case LAYER_NUMWORD:
            return "NWD";
        case LAYER_NUMBERS:
            return "NUM";
        case LAYER_NAVIGATION:
            return "NAV";
        case LAYER_FUNCTION:
            return "FN";
        case LAYER_MEDIA:
            return "MEDIA";
        case LAYER_POINTER:
            return "PTR";
        case LAYER_MACRO:
            return "MACRO";
        default:
            return "UNDEF";
    }
}

/*
 * The pointer page switches to explicit ON/OFF wording instead of compact
 * flags so that mode changes are easier to read at a glance.
 */
static const char *fleetsing_toggle_name(bool enabled) {
    return enabled ? "ON" : "OFF";
}

/* Keep OS-mode wording short so it fits the same compact field layout. */
static void fleetsing_format_os_mode(char *buffer, size_t size) {
    snprintf(buffer, size, "%s", fleetsing_get_os_mode_name());
}

/*
 * Render a field as a three-line block:
 * 1. label
 * 2. current value
 * 3. spacer line
 *
 * The blank line is intentional. It trades a bit of density for much faster
 * visual parsing on the small screen.
 */
static void fleetsing_render_pair(const char *label, const char *value) {
    oled_write_ln(label, false);
    oled_write_ln(value, false);
    oled_write_ln("", false);
}

static void fleetsing_render_top_padding(uint8_t lines) {
    for (uint8_t i = 0; i < lines; ++i) {
        oled_write_ln("", false);
    }
}

/*
 * Host LED state comes from the operating system rather than local userspace.
 *
 * The three letters map directly to Num Lock, Caps Lock, and Scroll Lock.
 * A dash means that particular host-side LED is off.
 */
static void fleetsing_format_host_leds(char *buffer, size_t size) {
    led_t led_state = host_keyboard_led_state();

    snprintf(buffer, size, "%c%c%c", led_state.num_lock ? 'N' : '-', led_state.caps_lock ? 'C' : '-', led_state.scroll_lock ? 'S' : '-');
}

/*
 * Friendly left/right wording is easier to scan on the pointer page and in
 * transient overlays than the compact single-letter form.
 */
static const char *fleetsing_scroll_side_name(void) {
#ifdef POINTING_DEVICE_COMBINED
    return fleetsing_get_scroll_side() == FLEETSING_SCROLL_SIDE_RIGHT ? "RIGHT" : "LEFT";
#else
    return "-";
#endif
}

/*
 * One-shot modifiers are rendered in a fixed Shift/Ctrl/Alt/GUI order.
 *
 * Lowercase means "armed for the next key". Uppercase means "locked until
 * cleared". This keeps transient modifier state visible without extra prose.
 */
static void fleetsing_format_oneshot_mods(char *buffer, size_t size) {
    const uint8_t active_mods = get_oneshot_mods();
    const uint8_t locked_mods = get_oneshot_locked_mods();

    /* Show one-shot mods as Shift/Ctrl/Alt/GUI, with case indicating armed vs locked. */
    snprintf(buffer, size, "%c%c%c%c", locked_mods & MOD_MASK_SHIFT ? 'S' : active_mods & MOD_MASK_SHIFT ? 's' : '-', locked_mods & MOD_MASK_CTRL ? 'C' : active_mods & MOD_MASK_CTRL ? 'c' : '-', locked_mods & MOD_MASK_ALT ? 'A' : active_mods & MOD_MASK_ALT ? 'a' : '-', locked_mods & MOD_MASK_GUI ? 'G' : active_mods & MOD_MASK_GUI ? 'g' : '-');
}

/*
 * Layer Lock is different from simply holding a layer key.
 *
 * This formatter only reports layers that were latched via QMK Layer Lock. If
 * nothing is latched, render a single "-" placeholder instead.
 */
static void fleetsing_format_locked_layers(char *buffer, size_t size) {
    size_t offset = 0;

    if (is_layer_locked(LAYER_NUMBERS)) {
        offset += snprintf(buffer + offset, size - offset, "%sNUM", offset ? "+" : "");
    }
    if (is_layer_locked(LAYER_NAVIGATION)) {
        offset += snprintf(buffer + offset, size - offset, "%sNAV", offset ? "+" : "");
    }
    if (is_layer_locked(LAYER_FUNCTION)) {
        offset += snprintf(buffer + offset, size - offset, "%sFN", offset ? "+" : "");
    }
    if (is_layer_locked(LAYER_MEDIA)) {
        offset += snprintf(buffer + offset, size - offset, "%sMEDIA", offset ? "+" : "");
    }
    if (is_layer_locked(LAYER_POINTER)) {
        offset += snprintf(buffer + offset, size - offset, "%sPTR", offset ? "+" : "");
    }
    if (is_layer_locked(LAYER_MACRO)) {
        offset += snprintf(buffer + offset, size - offset, "%sMAC", offset ? "+" : "");
    }

    if (offset == 0) {
        snprintf(buffer, size, "-");
    }
}

/*
 * Pointer flags summarize the two firmware-owned pointer modes:
 * - S = sniping mode enabled
 * - D = drag-scroll enabled
 *
 * Either flag becomes "-" when that mode is inactive.
 */
static void fleetsing_format_pointer_flags(char *buffer, size_t size) {
#ifdef POINTING_DEVICE_ENABLE
    snprintf(buffer, size, "%c%c", charybdis_get_pointer_sniping_enabled() ? 'S' : '-', charybdis_get_pointer_dragscroll_enabled() ? 'D' : '-');
#else
    snprintf(buffer, size, "--");
#endif
}

/*
 * Userspace chooses which sensor becomes the scroll source when both sensors
 * are active. Show that choice as a single character to conserve space.
 */
static void fleetsing_format_scroll_side(char *buffer, size_t size) {
#ifdef POINTING_DEVICE_COMBINED
    snprintf(buffer, size, "%c", fleetsing_get_scroll_side() == FLEETSING_SCROLL_SIDE_RIGHT ? 'R' : 'L');
#else
    snprintf(buffer, size, "-");
#endif
}

/*
 * Merge the pointer mode flags and scroll-side choice into the compact OLED
 * form used on the master screen, for example "S- L" or "-D R".
 */
static void fleetsing_format_pointer_mode(char *buffer, size_t size) {
    char flags[3];
    char side[2];

    /* Compress pointer state into a compact screen-friendly summary such as "S- L". */
    fleetsing_format_pointer_flags(flags, sizeof(flags));
    fleetsing_format_scroll_side(side, sizeof(side));
    snprintf(buffer, size, "%s %s", flags, side);
}

/*
 * Show the effective CPI currently applied to the trackball.
 *
 * This is mode-sensitive:
 * - drag-scroll forces its dedicated low DPI
 * - sniping uses the current sniping DPI
 * - otherwise the normal/default DPI is shown
 */
static uint16_t fleetsing_get_active_pointer_dpi(void) {
#ifdef POINTING_DEVICE_ENABLE
    if (charybdis_get_pointer_dragscroll_enabled()) {
        return CHARYBDIS_DRAGSCROLL_DPI;
    }

    if (charybdis_get_pointer_sniping_enabled()) {
        return charybdis_get_pointer_sniping_dpi();
    }

    return charybdis_get_pointer_default_dpi();
#else
    return 0;
#endif
}

/*
 * The pointer layer gets its own page because pointer state changes more
 * quickly than the normal dashboard and benefits from more explicit wording
 * than the compact default screen.
 */
static bool fleetsing_pointer_page_is_active(void) {
#ifdef POINTING_DEVICE_ENABLE
    return layer_state_cmp(layer_state, FLEETSING_AUTO_SNIPING_LAYER);
#else
    return false;
#endif
}

/*
 * Dedicated macro status is more useful as a temporary detail page than as a
 * single compact field because recording and playback are explicit modal tasks.
 */
static bool fleetsing_macro_page_is_active(void) {
#ifdef DYNAMIC_MACRO_ENABLE
    fleetsing_refresh_macro_status();
    return fleetsing_macro_status != FLEETSING_MACRO_IDLE;
#else
    return false;
#endif
}

/*
 * NumWord gets its own offhand page while active so the temporary timeout is
 * visible without crowding the steady-state dashboards.
 */
static bool fleetsing_numword_page_is_active(void) {
    return fleetsing_numword_display_is_active();
}

/* The NumWord timer should follow the physical right OLED, not USB mastership. */
static bool fleetsing_numword_page_on_this_half(void) {
    return !is_keyboard_left();
}

static void fleetsing_format_numword_remaining(char *buffer, size_t size) {
#if FLEETSING_NUMWORD_IDLE_TIMEOUT > 0
    uint16_t remaining = fleetsing_numword_display_remaining();
    uint8_t  tenths    = remaining / 100;
    snprintf(buffer, size, "%u.%us", tenths / 10, tenths % 10);
#else
    snprintf(buffer, size, "HOLD");
#endif
}

static void fleetsing_format_numword_progress(char *buffer, size_t size) {
#if FLEETSING_NUMWORD_IDLE_TIMEOUT > 0
    enum { bar_width = 8 };
    uint16_t remaining = fleetsing_numword_display_remaining();
    uint8_t  filled    = (uint8_t)(((uint32_t)remaining * bar_width + (FLEETSING_NUMWORD_IDLE_TIMEOUT - 1)) / FLEETSING_NUMWORD_IDLE_TIMEOUT);

    if (filled > bar_width) {
        filled = bar_width;
    }

    if (size < (size_t)(bar_width + 3)) {
        if (size > 0) {
            buffer[0] = '\0';
        }
        return;
    }

    buffer[0] = '[';
    for (uint8_t i = 0; i < bar_width; ++i) {
        buffer[i + 1] = i < filled ? '#' : '-';
    }
    buffer[bar_width + 1] = ']';
    buffer[bar_width + 2] = '\0';
#else
    snprintf(buffer, size, "[HOLD]");
#endif
}

/*
 * Split the compact macro status into a clearer action line for the dedicated
 * offhand page.
 */
static void fleetsing_format_macro_page_action(char *buffer, size_t size) {
#ifdef DYNAMIC_MACRO_ENABLE
    switch (fleetsing_macro_status) {
        case FLEETSING_MACRO_REC1:
        case FLEETSING_MACRO_REC2:
            snprintf(buffer, size, "RECORD");
            break;
        case FLEETSING_MACRO_PLAY1:
        case FLEETSING_MACRO_PLAY2:
            snprintf(buffer, size, "PLAY");
            break;
        case FLEETSING_MACRO_SAVE1:
        case FLEETSING_MACRO_SAVE2:
            snprintf(buffer, size, "STORED");
            break;
        default:
            snprintf(buffer, size, "-");
            break;
    }
#else
    snprintf(buffer, size, "-");
#endif
}

/*
 * Slot numbers are useful on the macro page because the base REC1/PLAY2 text
 * is concise but easy to skim past during quick modal work.
 */
static void fleetsing_format_macro_page_slot(char *buffer, size_t size) {
#ifdef DYNAMIC_MACRO_ENABLE
    switch (fleetsing_macro_status) {
        case FLEETSING_MACRO_REC1:
        case FLEETSING_MACRO_PLAY1:
        case FLEETSING_MACRO_SAVE1:
            snprintf(buffer, size, "1");
            break;
        case FLEETSING_MACRO_REC2:
        case FLEETSING_MACRO_PLAY2:
        case FLEETSING_MACRO_SAVE2:
            snprintf(buffer, size, "2");
            break;
        default:
            snprintf(buffer, size, "-");
            break;
    }
#else
    snprintf(buffer, size, "-");
#endif
}

/*
 * Treat Layer Lock, locked one-shot mods, and Caps Lock as notable enough to
 * deserve a warning-first offhand panel.
 *
 * The alert line is intentionally terse. More detail remains visible in the
 * surrounding fields so the offhand stays readable.
 */
static bool fleetsing_format_alert(char *buffer, size_t size) {
    char value[FLEETSING_OLED_VALUE_SIZE];

    fleetsing_format_locked_layers(value, sizeof(value));
    if (strcmp(value, "-") != 0) {
        snprintf(buffer, size, "LOCK %s", value);
        return true;
    }

    if (get_oneshot_locked_mods() != 0) {
        fleetsing_format_oneshot_mods(value, sizeof(value));
        snprintf(buffer, size, "OSM %s", value);
        return true;
    }

    if (host_keyboard_led_state().caps_lock) {
        snprintf(buffer, size, "CAPS ON");
        return true;
    }

    snprintf(buffer, size, "-");
    return false;
}

/*
 * Short-lived change overlays acknowledge mode transitions without making the
 * steady-state dashboards permanently denser.
 */
typedef struct {
    char title[10];
    char value[FLEETSING_OLED_VALUE_SIZE];
} fleetsing_oled_overlay_item_t;

typedef struct {
    bool                          initialized;
    bool                          active;
    uint8_t                       count;
    fleetsing_oled_overlay_item_t items[FLEETSING_OLED_OVERLAY_MAX_ITEMS];
    uint8_t                       last_layer;
    char                          last_lock[FLEETSING_OLED_VALUE_SIZE];
    char                          last_os[5];
    char                          last_osm[5];
    char                          last_scroll[6];
    uint16_t                      last_dpi;
    uint32_t                      timer;
} fleetsing_oled_overlay_state_t;

static fleetsing_oled_overlay_state_t fleetsing_oled_overlay_state = {0};

static void fleetsing_reset_oled_overlay_items(void) {
    fleetsing_oled_overlay_state.count = 0;
}

static void fleetsing_append_oled_overlay_item(const char *title, const char *value) {
    for (uint8_t i = 0; i < fleetsing_oled_overlay_state.count; ++i) {
        if (strcmp(fleetsing_oled_overlay_state.items[i].title, title) == 0) {
            snprintf(fleetsing_oled_overlay_state.items[i].value, sizeof(fleetsing_oled_overlay_state.items[i].value), "%s", value);
            return;
        }
    }

    if (fleetsing_oled_overlay_state.count >= FLEETSING_OLED_OVERLAY_MAX_ITEMS) {
        return;
    }

    snprintf(fleetsing_oled_overlay_state.items[fleetsing_oled_overlay_state.count].title, sizeof(fleetsing_oled_overlay_state.items[fleetsing_oled_overlay_state.count].title), "%s", title);
    snprintf(fleetsing_oled_overlay_state.items[fleetsing_oled_overlay_state.count].value, sizeof(fleetsing_oled_overlay_state.items[fleetsing_oled_overlay_state.count].value), "%s", value);
    fleetsing_oled_overlay_state.count++;
}

static bool fleetsing_overlay_is_active(void) {
    if (!fleetsing_oled_overlay_state.active) {
        return false;
    }

    if (timer_elapsed32(fleetsing_oled_overlay_state.timer) >= FLEETSING_OLED_OVERLAY_TIMEOUT) {
        fleetsing_oled_overlay_state.active = false;
        return false;
    }

    return true;
}

/*
 * Treat a short burst of related updates as one logical event.
 *
 * This keeps OLED feedback calmer when one action triggers several follow-on
 * state updates in quick succession.
 */
static bool fleetsing_should_reset_overlay_bundle(void) {
    return !fleetsing_oled_overlay_state.active || timer_elapsed32(fleetsing_oled_overlay_state.timer) >= FLEETSING_OLED_OVERLAY_GROUP_WINDOW;
}

/*
 * Detect changes in the most surprise-prone runtime state and convert them
 * into a short confirmation overlay.
 *
 * This runs from the display task instead of key hooks so state changes driven
 * by split sync or firmware-owned pointer settings still produce the same UI.
 */
static void fleetsing_update_oled_overlay(void) {
    char     lock[FLEETSING_OLED_VALUE_SIZE];
    char     os[5];
    char     osm[5];
    char     scroll[6];
    char     value[FLEETSING_OLED_VALUE_SIZE];
    uint8_t  layer       = get_highest_layer(layer_state);
    uint16_t dpi         = fleetsing_get_active_pointer_dpi();
    bool     has_changes = false;

    fleetsing_format_locked_layers(lock, sizeof(lock));
    fleetsing_format_os_mode(os, sizeof(os));
    fleetsing_format_oneshot_mods(osm, sizeof(osm));
    snprintf(scroll, sizeof(scroll), "%s", fleetsing_scroll_side_name());

    if (!fleetsing_oled_overlay_state.initialized) {
        fleetsing_oled_overlay_state.last_layer = layer;
        snprintf(fleetsing_oled_overlay_state.last_lock, sizeof(fleetsing_oled_overlay_state.last_lock), "%s", lock);
        snprintf(fleetsing_oled_overlay_state.last_os, sizeof(fleetsing_oled_overlay_state.last_os), "%s", os);
        snprintf(fleetsing_oled_overlay_state.last_osm, sizeof(fleetsing_oled_overlay_state.last_osm), "%s", osm);
        snprintf(fleetsing_oled_overlay_state.last_scroll, sizeof(fleetsing_oled_overlay_state.last_scroll), "%s", scroll);
        fleetsing_oled_overlay_state.last_dpi    = dpi;
        fleetsing_oled_overlay_state.initialized = true;
        fleetsing_oled_overlay_state.active      = false;
        return;
    }

    if (fleetsing_oled_overlay_state.last_layer != layer) {
        has_changes = true;
    }
    if (strcmp(fleetsing_oled_overlay_state.last_lock, lock) != 0) {
        has_changes = true;
    }
    if (strcmp(fleetsing_oled_overlay_state.last_os, os) != 0) {
        has_changes = true;
    }
    if (strcmp(fleetsing_oled_overlay_state.last_osm, osm) != 0) {
        has_changes = true;
    }
    if (strcmp(fleetsing_oled_overlay_state.last_scroll, scroll) != 0) {
        has_changes = true;
    }
    if (fleetsing_oled_overlay_state.last_dpi != dpi) {
        has_changes = true;
    }

    if (has_changes) {
        if (fleetsing_should_reset_overlay_bundle()) {
            fleetsing_reset_oled_overlay_items();
        }

        if (fleetsing_oled_overlay_state.last_layer != layer) {
            fleetsing_append_oled_overlay_item("Layer", fleetsing_layer_name(layer));
        }
        if (strcmp(fleetsing_oled_overlay_state.last_lock, lock) != 0) {
            snprintf(value, sizeof(value), "%s", strcmp(lock, "-") == 0 ? "CLEAR" : lock);
            fleetsing_append_oled_overlay_item("Lock", value);
        }
        if (strcmp(fleetsing_oled_overlay_state.last_os, os) != 0) {
            fleetsing_append_oled_overlay_item("OS", os);
        }
        if (strcmp(fleetsing_oled_overlay_state.last_osm, osm) != 0) {
            snprintf(value, sizeof(value), "%s", strcmp(osm, "----") == 0 ? "CLEAR" : osm);
            fleetsing_append_oled_overlay_item("OSM", value);
        }
        if (strcmp(fleetsing_oled_overlay_state.last_scroll, scroll) != 0) {
            fleetsing_append_oled_overlay_item("Scroll", scroll);
        }
        if (fleetsing_oled_overlay_state.last_dpi != dpi) {
            snprintf(value, sizeof(value), "%u", dpi);
            fleetsing_append_oled_overlay_item("DPI", value);
        }

        fleetsing_oled_overlay_state.timer  = timer_read32();
        fleetsing_oled_overlay_state.active = fleetsing_oled_overlay_state.count > 0;
    }

    fleetsing_oled_overlay_state.last_layer = layer;
    snprintf(fleetsing_oled_overlay_state.last_lock, sizeof(fleetsing_oled_overlay_state.last_lock), "%s", lock);
    snprintf(fleetsing_oled_overlay_state.last_os, sizeof(fleetsing_oled_overlay_state.last_os), "%s", os);
    snprintf(fleetsing_oled_overlay_state.last_osm, sizeof(fleetsing_oled_overlay_state.last_osm), "%s", osm);
    snprintf(fleetsing_oled_overlay_state.last_scroll, sizeof(fleetsing_oled_overlay_state.last_scroll), "%s", scroll);
    fleetsing_oled_overlay_state.last_dpi = dpi;
}

#ifdef DYNAMIC_MACRO_ENABLE
/*
 * Remember the most recent dynamic-macro state transition so the OLED can
 * acknowledge recording, saving, and playback without parsing macro internals.
 */
static void fleetsing_set_macro_status(fleetsing_macro_status_t status) {
    fleetsing_macro_status       = status;
    fleetsing_macro_status_timer = timer_read32();
}

/*
 * Recording remains visible until it stops explicitly. Playback and save
 * acknowledgements decay back to READY after a short confirmation window.
 */
static void fleetsing_refresh_macro_status(void) {
    if (fleetsing_macro_status != FLEETSING_MACRO_IDLE && timer_elapsed32(fleetsing_macro_status_timer) > 2000 && fleetsing_macro_status != FLEETSING_MACRO_REC1 && fleetsing_macro_status != FLEETSING_MACRO_REC2) {
        fleetsing_macro_status = FLEETSING_MACRO_IDLE;
    }
}

/*
 * Dynamic macros are event-driven, so the OLED keeps a tiny bit of recent
 * history:
 * - REC1 / REC2 stay visible while recording is active
 * - SAVE1 / SAVE2 and PLAY1 / PLAY2 are brief acknowledgements
 * - READY is the idle state
 */
static void fleetsing_format_macro_status(char *buffer, size_t size) {
    fleetsing_refresh_macro_status();

    switch (fleetsing_macro_status) {
        case FLEETSING_MACRO_REC1:
            snprintf(buffer, size, "REC1");
            break;
        case FLEETSING_MACRO_REC2:
            snprintf(buffer, size, "REC2");
            break;
        case FLEETSING_MACRO_PLAY1:
            snprintf(buffer, size, "PLAY1");
            break;
        case FLEETSING_MACRO_PLAY2:
            snprintf(buffer, size, "PLAY2");
            break;
        case FLEETSING_MACRO_SAVE1:
            snprintf(buffer, size, "SAVE1");
            break;
        case FLEETSING_MACRO_SAVE2:
            snprintf(buffer, size, "SAVE2");
            break;
        default:
            snprintf(buffer, size, "READY");
            break;
    }
}

/* Start recording into macro slot 1 or 2 and reflect that immediately on screen. */
bool dynamic_macro_record_start_user(int8_t direction) {
    dynamic_macro_led_blink();
    fleetsing_set_macro_status(direction > 0 ? FLEETSING_MACRO_REC1 : FLEETSING_MACRO_REC2);
    fleetsing_haptic_play_event(FLEETSING_HAPTIC_MACRO_RECORD_START);
    return true;
}

/* Keep QMK's normal behavior; this hook exists only to preserve blink feedback. */
bool dynamic_macro_record_key_user(int8_t direction, keyrecord_t *record) {
    (void)direction;
    (void)record;

    dynamic_macro_led_blink();
    return true;
}

/* Recording has ended and the slot contents were saved successfully. */
bool dynamic_macro_record_end_user(int8_t direction) {
    dynamic_macro_led_blink();
    fleetsing_set_macro_status(direction > 0 ? FLEETSING_MACRO_SAVE1 : FLEETSING_MACRO_SAVE2);
    fleetsing_haptic_play_event(FLEETSING_HAPTIC_MACRO_RECORD_STOP);
    return true;
}

/* Playback of macro slot 1 or 2 was triggered. */
bool dynamic_macro_play_user(int8_t direction) {
    dynamic_macro_led_blink();
    fleetsing_set_macro_status(direction > 0 ? FLEETSING_MACRO_PLAY1 : FLEETSING_MACRO_PLAY2);
    fleetsing_haptic_play_event(FLEETSING_HAPTIC_MACRO_PLAY);
    return true;
}
#else
/* Dynamic macros are not enabled for this build, so show a neutral placeholder. */
static void fleetsing_format_macro_status(char *buffer, size_t size) {
    snprintf(buffer, size, "-");
}
#endif

/*
 * The master half is the active-status dashboard.
 *
 * These fields are the ones most likely to matter while actively using the
 * board: current layer, latched layers, pointer mode, and effective DPI.
 */
static void fleetsing_render_master_panel(void) {
    char value[FLEETSING_OLED_VALUE_SIZE];

    fleetsing_render_top_padding(FLEETSING_OLED_MASTER_TOP_PAD);
    fleetsing_render_pair("Layer", fleetsing_layer_name(get_highest_layer(layer_state)));
    fleetsing_format_locked_layers(value, sizeof(value));
    fleetsing_render_pair("Lock", value);

    fleetsing_format_pointer_mode(value, sizeof(value));
    fleetsing_render_pair("Ptr", value);

    snprintf(value, sizeof(value), "%u", fleetsing_get_active_pointer_dpi());
    fleetsing_render_pair("DPI", value);
}

/*
 * The pointer page trades slower offhand status for direct pointer controls
 * and state. It appears automatically on the secondary half while the pointer
 * layer is active.
 */
static void fleetsing_render_pointer_panel(void) {
    char value[FLEETSING_OLED_VALUE_SIZE];

    fleetsing_render_top_padding(FLEETSING_OLED_TEMP_TOP_PAD);
#ifdef POINTING_DEVICE_ENABLE
    fleetsing_render_pair("Snip", fleetsing_toggle_name(charybdis_get_pointer_sniping_enabled()));
    fleetsing_render_pair("Drag", fleetsing_toggle_name(charybdis_get_pointer_dragscroll_enabled()));
#else
    fleetsing_render_pair("Snip", "OFF");
    fleetsing_render_pair("Drag", "OFF");
#endif
    fleetsing_render_pair("Scroll", fleetsing_scroll_side_name());

    snprintf(value, sizeof(value), "%u", fleetsing_get_active_pointer_dpi());
    fleetsing_render_pair("DPI", value);
}

/*
 * Macro actions use a dedicated temporary offhand page because they are
 * explicit modal tasks that benefit from more context than the compact field.
 */
static void fleetsing_render_macro_panel(void) {
    char value[FLEETSING_OLED_VALUE_SIZE];

    fleetsing_render_top_padding(FLEETSING_OLED_TEMP_TOP_PAD);
    fleetsing_format_macro_status(value, sizeof(value));
    fleetsing_render_pair("Macro", value);

    fleetsing_format_macro_page_action(value, sizeof(value));
    fleetsing_render_pair("State", value);

    fleetsing_format_macro_page_slot(value, sizeof(value));
    fleetsing_render_pair("Slot", value);
}

/*
 * This page lives on the physical right OLED so the timeout appears in a stable
 * place regardless of which half currently owns USB mastership. The displayed
 * remaining time is synced from the master-side NumWord state.
 */
static void fleetsing_render_numword_panel(void) {
    char value[FLEETSING_OLED_VALUE_SIZE];

    fleetsing_render_top_padding(FLEETSING_OLED_TEMP_TOP_PAD);
    fleetsing_render_pair("NumWord", "ON");
    fleetsing_format_numword_remaining(value, sizeof(value));
    fleetsing_render_pair("Exit In", value);
    fleetsing_format_numword_progress(value, sizeof(value));
    oled_write_ln(value, false);
    oled_write_ln("", false);
    fleetsing_render_pair("Lock", "NUM");
}

/*
 * Overlays are intentionally simple and live on the non-master side so the
 * master display can keep its stable layer-oriented overview. When several
 * related states change together, bundle them into the same short-lived page.
 */
static void fleetsing_render_overlay(void) {
    fleetsing_render_top_padding(FLEETSING_OLED_TEMP_TOP_PAD);
    oled_write_ln("Changed", false);
    oled_write_ln("", false);

    for (uint8_t i = 0; i < fleetsing_oled_overlay_state.count; ++i) {
        fleetsing_render_pair(fleetsing_oled_overlay_state.items[i].title, fleetsing_oled_overlay_state.items[i].value);
    }
}

/*
 * The offhand half is the lower-noise secondary panel.
 *
 * It intentionally omits the current layer and instead focuses on slower or
 * more situational state: OS mode, macro status, and host LEDs.
 *
 * By convention, temporary or mode-specific detail pages should prefer this
 * side so the master half keeps its stable layer-oriented overview.
 */
static void fleetsing_render_offhand_panel(void) {
    char value[FLEETSING_OLED_VALUE_SIZE];

    /*
     * Offhand side is the lower-churn status panel.
     *
     * Priority order on the offhand side:
     * 1. dedicated temporary macro page
     * 2. pointer page while the pointer layer is active
     * 3. NumWord countdown on the physical right half while the layer is active
     * 4. warning-first status page for unusual conditions
     * 5. normal low-churn status page
     */
    if (fleetsing_macro_page_is_active()) {
        fleetsing_render_macro_panel();
        return;
    }

    if (fleetsing_pointer_page_is_active()) {
        fleetsing_render_pointer_panel();
        return;
    }

    if (fleetsing_numword_page_is_active() && fleetsing_numword_page_on_this_half()) {
        fleetsing_render_numword_panel();
        return;
    }

    fleetsing_render_top_padding(FLEETSING_OLED_OFFHAND_TOP_PAD);

    if (fleetsing_format_alert(value, sizeof(value))) {
        fleetsing_render_pair("Alert", value);
        fleetsing_format_os_mode(value, sizeof(value));
        fleetsing_render_pair("OS", value);
        fleetsing_format_host_leds(value, sizeof(value));
        fleetsing_render_pair("Host", value);
        return;
    }

    fleetsing_format_os_mode(value, sizeof(value));
    fleetsing_render_pair("OS", value);

    fleetsing_format_macro_status(value, sizeof(value));
    fleetsing_render_pair("Macro", value);

    fleetsing_format_host_leds(value, sizeof(value));
    fleetsing_render_pair("Host", value);
}

/* The displays are physically mounted upside down relative to QMK defaults. */
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_180;
}

#ifdef OLED_ENABLE
/*
 * Cache the last rendered screen contents locally on each half.
 *
 * Even though userspace now owns the OLED sleep timer, avoiding identical
 * redraws is still worthwhile: it reduces needless display churn, keeps wake
 * behavior easier to reason about, and makes it clearer when a real state
 * change forced a refresh.
 */
static bool fleetsing_update_oled_snapshot(const char *snapshot) {
    static char last_snapshot[FLEETSING_OLED_SNAPSHOT_SIZE] = "";

    if (strcmp(last_snapshot, snapshot) == 0) {
        return false;
    }

    snprintf(last_snapshot, sizeof(last_snapshot), "%s", snapshot);
    return true;
}

static void fleetsing_invalidate_oled_snapshot(void) {
    fleetsing_update_oled_snapshot("");
}

/*
 * Keep a userspace-owned notion of "recent OLED activity".
 *
 * This deliberately follows explicit key activity rather than QMK's broader
 * activity timestamps so split pointing noise cannot keep the displays awake.
 */
static uint32_t fleetsing_oled_activity_timer = 0;

void fleetsing_display_note_activity(void) {
    fleetsing_oled_activity_timer = timer_read32();
}

/*
 * OLED sleep only depends on the elapsed time since the last explicit
 * activity note from key or pointing hooks.
 */
static uint32_t fleetsing_display_idle_elapsed(void) {
    return timer_elapsed32(fleetsing_oled_activity_timer);
}

/*
 * QMK calls this repeatedly to redraw the display buffer.
 *
 * Clear the screen each frame and render the appropriate panel for the current
 * half so the two OLEDs can present different information.
 */
bool oled_task_user(void) {
    char snapshot[FLEETSING_OLED_SNAPSHOT_SIZE];

    fleetsing_update_oled_overlay();

    if (is_keyboard_master()) {
        /*
         * Use the userspace-owned key-activity timer for OLED sleep/wake so
         * split trackball chatter cannot keep the displays alive forever.
         * The master half owns the power state and split OLED sync mirrors it
         * to the other side.
         */
        if (fleetsing_display_idle_elapsed() >= FLEETSING_OLED_IDLE_TIMEOUT) {
            if (is_oled_on()) {
                oled_off();
            }
            fleetsing_invalidate_oled_snapshot();
            return false;
        }

        if (!is_oled_on()) {
            oled_on();
            fleetsing_invalidate_oled_snapshot();
        }
    } else if (!is_oled_on()) {
        fleetsing_invalidate_oled_snapshot();
        return false;
    }

    if (!is_keyboard_master() && fleetsing_overlay_is_active()) {
        size_t offset = snprintf(snapshot, sizeof(snapshot), "X");

        for (uint8_t i = 0; i < fleetsing_oled_overlay_state.count && offset < sizeof(snapshot); ++i) {
            offset += snprintf(snapshot + offset, sizeof(snapshot) - offset, "|%s|%s", fleetsing_oled_overlay_state.items[i].title, fleetsing_oled_overlay_state.items[i].value);
        }

        if (!fleetsing_update_oled_snapshot(snapshot)) {
            return false;
        }

        oled_clear();
        oled_set_cursor(0, 0);
        fleetsing_render_overlay();
        return false;
    }

    if (fleetsing_numword_page_is_active() && fleetsing_numword_page_on_this_half()) {
        char field_a[FLEETSING_OLED_VALUE_SIZE];
        fleetsing_format_numword_remaining(field_a, sizeof(field_a));
        snprintf(snapshot, sizeof(snapshot), "W|%s", field_a);

        if (!fleetsing_update_oled_snapshot(snapshot)) {
            return false;
        }

        oled_clear();
        oled_set_cursor(0, 0);
        fleetsing_render_numword_panel();
    } else if (is_keyboard_master()) {
        char field_a[FLEETSING_OLED_VALUE_SIZE];
        char field_b[FLEETSING_OLED_VALUE_SIZE];
        char field_c[FLEETSING_OLED_VALUE_SIZE];
        fleetsing_format_locked_layers(field_a, sizeof(field_a));
        fleetsing_format_pointer_mode(field_b, sizeof(field_b));
        snprintf(field_c, sizeof(field_c), "%u", fleetsing_get_active_pointer_dpi());
        snprintf(snapshot, sizeof(snapshot), "M|%s|%s|%s|%s", fleetsing_layer_name(get_highest_layer(layer_state)), field_a, field_b, field_c);

        if (!fleetsing_update_oled_snapshot(snapshot)) {
            return false;
        }

        oled_clear();
        oled_set_cursor(0, 0);
        fleetsing_render_master_panel();
    } else {
        char alert[FLEETSING_OLED_VALUE_SIZE];
        char os[FLEETSING_OLED_VALUE_SIZE];
        char field_c[FLEETSING_OLED_VALUE_SIZE];
        char host[FLEETSING_OLED_VALUE_SIZE];
        bool has_alert = fleetsing_format_alert(alert, sizeof(alert));

        if (fleetsing_macro_page_is_active()) {
            fleetsing_format_macro_status(alert, sizeof(alert));
            fleetsing_format_macro_page_action(os, sizeof(os));
            fleetsing_format_macro_page_slot(field_c, sizeof(field_c));
            snprintf(snapshot, sizeof(snapshot), "O|M|%s|%s|%s", alert, os, field_c);
        } else if (fleetsing_pointer_page_is_active()) {
            snprintf(snapshot, sizeof(snapshot), "O|P|%s|%s|%s|%u", fleetsing_toggle_name(charybdis_get_pointer_sniping_enabled()), fleetsing_toggle_name(charybdis_get_pointer_dragscroll_enabled()), fleetsing_scroll_side_name(), fleetsing_get_active_pointer_dpi());
        } else if (fleetsing_numword_page_is_active() && fleetsing_numword_page_on_this_half()) {
            fleetsing_format_numword_remaining(field_c, sizeof(field_c));
            snprintf(snapshot, sizeof(snapshot), "O|W|%s", field_c);
        } else if (has_alert) {
            fleetsing_format_os_mode(field_c, sizeof(field_c));
            fleetsing_format_host_leds(host, sizeof(host));
            snprintf(snapshot, sizeof(snapshot), "O|A|%s|%s|%s", alert, field_c, host);
        } else {
            fleetsing_format_os_mode(os, sizeof(os));
            fleetsing_format_macro_status(field_c, sizeof(field_c));
            fleetsing_format_host_leds(host, sizeof(host));
            snprintf(snapshot, sizeof(snapshot), "O|N|%s|%s|%s", os, field_c, host);
        }

        if (!fleetsing_update_oled_snapshot(snapshot)) {
            return false;
        }

        oled_clear();
        oled_set_cursor(0, 0);
        fleetsing_render_offhand_panel();
    }

    return false;
}
#endif

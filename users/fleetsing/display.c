#include "fleetsing.h"

#include <stdio.h>
#include <string.h>

#ifndef CHARYBDIS_DRAGSCROLL_DPI
#    define CHARYBDIS_DRAGSCROLL_DPI 100
#endif

#ifndef FLEETSING_OLED_IDLE_TIMEOUT
#    define FLEETSING_OLED_IDLE_TIMEOUT 30000
#endif

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
#endif

/*
 * OLED glossary:
 * - Layer: highest currently active layer on the master half.
 * - Lock: layers latched with QMK Layer Lock, not merely held.
 * - Ptr: two mode flags plus the selected scroll source, shown as "<SD> <L/R>".
 *        S = sniping, D = drag-scroll, L/R = left/right sensor becomes scroll.
 * - DPI: effective pointer CPI for the current pointer mode.
 * - Macro: dynamic macro recorder/playback status for slot 1 or 2.
 * - OSM: one-shot modifiers in Shift, Ctrl, Alt, GUI order.
 *        Lowercase means armed for the next key, uppercase means locked.
 * - Host: host keyboard LED state in Num, Caps, Scroll order.
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
 * Dynamic macros are event-driven, so the OLED keeps a tiny bit of recent
 * history:
 * - REC1 / REC2 stay visible while recording is active
 * - SAVE1 / SAVE2 and PLAY1 / PLAY2 are brief acknowledgements
 * - READY is the idle state
 */
static void fleetsing_format_macro_status(char *buffer, size_t size) {
    /*
     * Recording states stay visible until recording ends.
     * Playback/save states are transient acknowledgements that decay back to READY.
     */
    if (fleetsing_macro_status != FLEETSING_MACRO_IDLE && timer_elapsed32(fleetsing_macro_status_timer) > 2000 && fleetsing_macro_status != FLEETSING_MACRO_REC1 && fleetsing_macro_status != FLEETSING_MACRO_REC2) {
        fleetsing_macro_status = FLEETSING_MACRO_IDLE;
    }

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
    return true;
}

/* Playback of macro slot 1 or 2 was triggered. */
bool dynamic_macro_play_user(int8_t direction) {
    dynamic_macro_led_blink();
    fleetsing_set_macro_status(direction > 0 ? FLEETSING_MACRO_PLAY1 : FLEETSING_MACRO_PLAY2);
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
    char value[12];

    fleetsing_render_pair("Layer", fleetsing_layer_name(get_highest_layer(layer_state)));
    fleetsing_format_locked_layers(value, sizeof(value));
    fleetsing_render_pair("Lock", value);

    fleetsing_format_pointer_mode(value, sizeof(value));
    fleetsing_render_pair("Ptr", value);

    snprintf(value, sizeof(value), "%u", fleetsing_get_active_pointer_dpi());
    fleetsing_render_pair("DPI", value);
}

/*
 * The offhand half is the lower-noise secondary panel.
 *
 * It intentionally omits the current layer and instead focuses on slower or
 * more situational state: macro status, one-shot mods, and host LEDs.
 */
static void fleetsing_render_offhand_panel(void) {
    char value[12];

    /*
     * Offhand side is the lower-churn status panel.
     * It omits the current layer so the content stays sparse and easier to scan.
     */
    fleetsing_format_macro_status(value, sizeof(value));
    fleetsing_render_pair("Macro", value);

    fleetsing_format_oneshot_mods(value, sizeof(value));
    fleetsing_render_pair("OSM", value);

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
    static char last_snapshot[96] = "";

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
    char snapshot[96];

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

    if (is_keyboard_master()) {
        char field_a[12];
        char field_b[12];
        char field_c[12];

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
        char macro[12];
        char osm[12];
        char host[12];

        fleetsing_format_macro_status(macro, sizeof(macro));
        fleetsing_format_oneshot_mods(osm, sizeof(osm));
        fleetsing_format_host_leds(host, sizeof(host));
        snprintf(snapshot, sizeof(snapshot), "O|%s|%s|%s", macro, osm, host);

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

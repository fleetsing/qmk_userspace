#include "fleetsing.h"

#include <stdio.h>
#include <string.h>

#ifdef SPLIT_TRANSACTION_IDS_USER
#    include "transactions.h"
#endif

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
#define FLEETSING_OLED_LEFT_INSET 1

typedef struct {
    char title[10];
    char value[FLEETSING_OLED_VALUE_SIZE];
} fleetsing_oled_overlay_item_t;

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

#ifdef SPLIT_TRANSACTION_IDS_USER
typedef struct {
    uint8_t                       layer;
    uint8_t                       locked_layers_mask;
    uint8_t                       pointer_flags;
    uint16_t                      dpi;
    uint8_t                       os_mode;
    uint8_t                       macro_status;
    uint8_t                       host_leds;
    uint8_t                       oneshot_mods;
    uint8_t                       oneshot_locked_mods;
    bool                          overlay_active;
    uint8_t                       overlay_count;
    fleetsing_oled_overlay_item_t overlay_items[FLEETSING_OLED_OVERLAY_MAX_ITEMS];
} fleetsing_display_sync_t;

static fleetsing_display_sync_t fleetsing_display_remote_state = {0};

_Static_assert(sizeof(fleetsing_display_sync_t) <= RPC_M2S_BUFFER_SIZE, "Display sync state exceeds split RPC master-to-slave buffer");
#endif

/*
 * OLED glossary:
 * - Layer: highest currently active layer on the physical left-side dashboard.
 * - Lock: layers latched with QMK Layer Lock, not merely held.
 * - Ptr: two mode flags, shown as "<SD>".
 *        S = sniping, D = drag-scroll.
 * - DPI: effective pointer CPI for the current pointer mode.
 * - OS: persisted shortcut/symbol mode. MAC keeps Cmd-native behavior, while
 *       PC swaps Ctrl/GUI so the Command-position keys behave more like Ctrl.
 * - Macro: dynamic macro recorder/playback status for slot 1 or 2.
 * - OSM: one-shot modifiers in Shift, Ctrl, Alt, GUI order.
 *        Lowercase means armed for the next key, uppercase means locked.
 * - Host: host keyboard LED state in Num, Caps, Scroll order.
 * - Alert: a right-side warning-first summary for unusual active conditions.
 * - Overlay: a short-lived bundled confirmation page that can show several
 *            changed states together, such as layer + DPI on pointer-layer entry.
 * - Macro Page: a temporary right-side detail page during macro recording,
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
        case LAYER_SYMBOLS:
            return "SYM";
        case LAYER_MEDIA:
            return "MEDIA";
        case LAYER_POINTER:
            return "PTR";
        case LAYER_MACRO:
            return "MACRO";
        case LAYER_SCROLL_LEFT:
        case LAYER_SCROLL_RIGHT:
            return "SCR";
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

static void fleetsing_format_os_mode_state(uint8_t mode, char *buffer, size_t size) {
    snprintf(buffer, size, "%s", mode == FLEETSING_OS_PC ? "PC" : "MAC");
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
static void fleetsing_render_line(const char *text) {
    for (uint8_t i = 0; i < FLEETSING_OLED_LEFT_INSET; ++i) {
        oled_write(" ", false);
    }
    oled_write_ln(text, false);
}

static void fleetsing_render_pair(const char *label, const char *value) {
    fleetsing_render_line(label);
    fleetsing_render_line(value);
    fleetsing_render_line("");
}

static void fleetsing_render_top_padding(uint8_t lines) {
    for (uint8_t i = 0; i < lines; ++i) {
        fleetsing_render_line("");
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

static void fleetsing_format_host_leds_state(uint8_t led_bits, char *buffer, size_t size) {
    snprintf(buffer, size, "%c%c%c", led_bits & 0x01 ? 'N' : '-', led_bits & 0x02 ? 'C' : '-', led_bits & 0x04 ? 'S' : '-');
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

static void fleetsing_format_oneshot_mods_state(uint8_t active_mods, uint8_t locked_mods, char *buffer, size_t size) {
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
    if (is_layer_locked(LAYER_SYMBOLS)) {
        offset += snprintf(buffer + offset, size - offset, "%sSYM", offset ? "+" : "");
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

static uint8_t fleetsing_locked_layers_mask(void) {
    uint8_t mask = 0;

    if (is_layer_locked(LAYER_NUMBERS)) {
        mask |= (uint8_t)1 << 0;
    }
    if (is_layer_locked(LAYER_NAVIGATION)) {
        mask |= (uint8_t)1 << 1;
    }
    if (is_layer_locked(LAYER_FUNCTION)) {
        mask |= (uint8_t)1 << 2;
    }
    if (is_layer_locked(LAYER_SYMBOLS)) {
        mask |= (uint8_t)1 << 3;
    }
    if (is_layer_locked(LAYER_MEDIA)) {
        mask |= (uint8_t)1 << 4;
    }
    if (is_layer_locked(LAYER_POINTER)) {
        mask |= (uint8_t)1 << 5;
    }
    if (is_layer_locked(LAYER_MACRO)) {
        mask |= (uint8_t)1 << 6;
    }

    return mask;
}

static void fleetsing_format_locked_layers_state(uint8_t mask, char *buffer, size_t size) {
    size_t offset = 0;

    if (mask & ((uint8_t)1 << 0)) {
        offset += snprintf(buffer + offset, size - offset, "%sNUM", offset ? "+" : "");
    }
    if (mask & ((uint8_t)1 << 1)) {
        offset += snprintf(buffer + offset, size - offset, "%sNAV", offset ? "+" : "");
    }
    if (mask & ((uint8_t)1 << 2)) {
        offset += snprintf(buffer + offset, size - offset, "%sFN", offset ? "+" : "");
    }
    if (mask & ((uint8_t)1 << 3)) {
        offset += snprintf(buffer + offset, size - offset, "%sSYM", offset ? "+" : "");
    }
    if (mask & ((uint8_t)1 << 4)) {
        offset += snprintf(buffer + offset, size - offset, "%sMEDIA", offset ? "+" : "");
    }
    if (mask & ((uint8_t)1 << 5)) {
        offset += snprintf(buffer + offset, size - offset, "%sPTR", offset ? "+" : "");
    }
    if (mask & ((uint8_t)1 << 6)) {
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

/* Render the compact two-flag pointer status used on the dashboard. */
static void fleetsing_format_pointer_mode(char *buffer, size_t size) {
    fleetsing_format_pointer_flags(buffer, size);
}

static uint8_t fleetsing_pointer_flags_state(void) {
    uint8_t flags = 0;
#ifdef POINTING_DEVICE_ENABLE
    if (charybdis_get_pointer_sniping_enabled()) {
        flags |= (uint8_t)1 << 0;
    }
    if (charybdis_get_pointer_dragscroll_enabled()) {
        flags |= (uint8_t)1 << 1;
    }
#endif
    return flags;
}

static void fleetsing_format_pointer_mode_state(uint8_t flags, char *buffer, size_t size) {
    snprintf(buffer, size, "%c%c", flags & ((uint8_t)1 << 0) ? 'S' : '-', flags & ((uint8_t)1 << 1) ? 'D' : '-');
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
 * NumWord gets its own right-side page while active so the temporary timeout is
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
 * right-side page.
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

static void fleetsing_format_macro_page_action_state(uint8_t status, char *buffer, size_t size) {
#ifdef DYNAMIC_MACRO_ENABLE
    switch ((fleetsing_macro_status_t)status) {
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

static void fleetsing_format_macro_page_slot_state(uint8_t status, char *buffer, size_t size) {
#ifdef DYNAMIC_MACRO_ENABLE
    switch ((fleetsing_macro_status_t)status) {
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
 * deserve a warning-first right-side panel.
 *
 * The alert line is intentionally terse. More detail remains visible in the
 * surrounding fields so the right-side panel stays readable.
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

static bool fleetsing_format_alert_state(uint8_t locked_layers_mask, uint8_t oneshot_mods, uint8_t oneshot_locked_mods, uint8_t host_leds, char *buffer, size_t size) {
    char value[FLEETSING_OLED_VALUE_SIZE];

    fleetsing_format_locked_layers_state(locked_layers_mask, value, sizeof(value));
    if (strcmp(value, "-") != 0) {
        snprintf(buffer, size, "LOCK %s", value);
        return true;
    }

    if (oneshot_locked_mods != 0) {
        fleetsing_format_oneshot_mods_state(oneshot_mods, oneshot_locked_mods, value, sizeof(value));
        snprintf(buffer, size, "OSM %s", value);
        return true;
    }

    if (host_leds & 0x02) {
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
    bool                          initialized;
    bool                          active;
    uint8_t                       count;
    fleetsing_oled_overlay_item_t items[FLEETSING_OLED_OVERLAY_MAX_ITEMS];
    uint8_t                       last_layer;
    char                          last_lock[FLEETSING_OLED_VALUE_SIZE];
    char                          last_os[5];
    char                          last_osm[5];
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

static bool fleetsing_local_overlay_is_active(void) {
    if (!fleetsing_oled_overlay_state.active) {
        return false;
    }

    if (timer_elapsed32(fleetsing_oled_overlay_state.timer) >= FLEETSING_OLED_OVERLAY_TIMEOUT) {
        fleetsing_oled_overlay_state.active = false;
        return false;
    }

    return true;
}

static bool fleetsing_overlay_is_active(void) {
#ifdef SPLIT_TRANSACTION_IDS_USER
    if (!is_keyboard_master() && !is_keyboard_left()) {
        return fleetsing_display_remote_state.overlay_active;
    }
#endif
    return fleetsing_local_overlay_is_active();
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
    char     value[FLEETSING_OLED_VALUE_SIZE];
    uint8_t  layer       = get_highest_layer(layer_state);
    uint16_t dpi         = fleetsing_get_active_pointer_dpi();
    bool     has_changes = false;

    fleetsing_format_locked_layers(lock, sizeof(lock));
    fleetsing_format_os_mode(os, sizeof(os));
    fleetsing_format_oneshot_mods(osm, sizeof(osm));

    if (!fleetsing_oled_overlay_state.initialized) {
        fleetsing_oled_overlay_state.last_layer = layer;
        snprintf(fleetsing_oled_overlay_state.last_lock, sizeof(fleetsing_oled_overlay_state.last_lock), "%s", lock);
        snprintf(fleetsing_oled_overlay_state.last_os, sizeof(fleetsing_oled_overlay_state.last_os), "%s", os);
        snprintf(fleetsing_oled_overlay_state.last_osm, sizeof(fleetsing_oled_overlay_state.last_osm), "%s", osm);
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

static void fleetsing_format_macro_status_state(uint8_t status, char *buffer, size_t size) {
    switch ((fleetsing_macro_status_t)status) {
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

static void fleetsing_format_macro_status_state(uint8_t status, char *buffer, size_t size) {
    (void)status;
    snprintf(buffer, size, "-");
}
#endif

/*
 * The physical left half is the active-status dashboard.
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

static void fleetsing_render_left_panel_remote(void) {
#ifdef SPLIT_TRANSACTION_IDS_USER
    char value[FLEETSING_OLED_VALUE_SIZE];

    fleetsing_render_top_padding(FLEETSING_OLED_MASTER_TOP_PAD);
    fleetsing_render_pair("Layer", fleetsing_layer_name(fleetsing_display_remote_state.layer));
    fleetsing_format_locked_layers_state(fleetsing_display_remote_state.locked_layers_mask, value, sizeof(value));
    fleetsing_render_pair("Lock", value);
    fleetsing_format_pointer_mode_state(fleetsing_display_remote_state.pointer_flags, value, sizeof(value));
    fleetsing_render_pair("Ptr", value);
    snprintf(value, sizeof(value), "%u", fleetsing_display_remote_state.dpi);
    fleetsing_render_pair("DPI", value);
#endif
}

/*
 * The pointer page trades slower right-side status for direct pointer controls
 * and state. It appears automatically on the physical right half while the
 * pointer layer is active.
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
    snprintf(value, sizeof(value), "%u", fleetsing_get_active_pointer_dpi());
    fleetsing_render_pair("DPI", value);
}

static void fleetsing_render_pointer_panel_remote(void) {
#ifdef SPLIT_TRANSACTION_IDS_USER
    char value[FLEETSING_OLED_VALUE_SIZE];

    fleetsing_render_top_padding(FLEETSING_OLED_TEMP_TOP_PAD);
    fleetsing_render_pair("Snip", fleetsing_toggle_name((fleetsing_display_remote_state.pointer_flags & ((uint8_t)1 << 0)) != 0));
    fleetsing_render_pair("Drag", fleetsing_toggle_name((fleetsing_display_remote_state.pointer_flags & ((uint8_t)1 << 1)) != 0));
    snprintf(value, sizeof(value), "%u", fleetsing_display_remote_state.dpi);
    fleetsing_render_pair("DPI", value);
#endif
}

/*
 * Macro actions use a dedicated temporary right-side page because they are
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

static void fleetsing_render_macro_panel_remote(void) {
#ifdef SPLIT_TRANSACTION_IDS_USER
    char value[FLEETSING_OLED_VALUE_SIZE];

    fleetsing_render_top_padding(FLEETSING_OLED_TEMP_TOP_PAD);
    fleetsing_format_macro_status_state(fleetsing_display_remote_state.macro_status, value, sizeof(value));
    fleetsing_render_pair("Macro", value);
    fleetsing_format_macro_page_action_state(fleetsing_display_remote_state.macro_status, value, sizeof(value));
    fleetsing_render_pair("State", value);
    fleetsing_format_macro_page_slot_state(fleetsing_display_remote_state.macro_status, value, sizeof(value));
    fleetsing_render_pair("Slot", value);
#endif
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
    /* Keep the full-width bar flush-left so the added display inset does not wrap it. */
    oled_write_ln(value, false);
    fleetsing_render_line("");
    fleetsing_render_pair("Lock", "NUM");
}

/*
 * Overlays are intentionally simple and live on the physical right side so the
 * left-side dashboard can keep its stable layer-oriented overview. When several
 * related states change together, bundle them into the same short-lived page.
 */
static void fleetsing_render_overlay(void) {
    fleetsing_render_top_padding(FLEETSING_OLED_TEMP_TOP_PAD);
    fleetsing_render_line("Changed");
    fleetsing_render_line("");

    for (uint8_t i = 0; i < fleetsing_oled_overlay_state.count; ++i) {
        fleetsing_render_pair(fleetsing_oled_overlay_state.items[i].title, fleetsing_oled_overlay_state.items[i].value);
    }
}

static void fleetsing_render_overlay_remote(void) {
#ifdef SPLIT_TRANSACTION_IDS_USER
    fleetsing_render_top_padding(FLEETSING_OLED_TEMP_TOP_PAD);
    fleetsing_render_line("Changed");
    fleetsing_render_line("");

    for (uint8_t i = 0; i < fleetsing_display_remote_state.overlay_count; ++i) {
        fleetsing_render_pair(fleetsing_display_remote_state.overlay_items[i].title, fleetsing_display_remote_state.overlay_items[i].value);
    }
#endif
}

/*
 * The physical right half is the lower-noise secondary panel.
 *
 * It intentionally omits the current layer and instead focuses on slower or
 * more situational state: OS mode, macro status, and host LEDs.
 *
 * By convention, temporary or mode-specific detail pages should prefer this
 * side so the left half keeps its stable layer-oriented overview.
 */
static void fleetsing_render_offhand_panel(void) {
    char value[FLEETSING_OLED_VALUE_SIZE];

    /*
     * Right side is the lower-churn status panel.
     *
     * Priority order on the right side:
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

static void fleetsing_render_right_panel_remote(void) {
#ifdef SPLIT_TRANSACTION_IDS_USER
    char value[FLEETSING_OLED_VALUE_SIZE];

    if ((fleetsing_macro_page_is_active() || fleetsing_display_remote_state.macro_status != FLEETSING_MACRO_IDLE)) {
        fleetsing_render_macro_panel_remote();
        return;
    }

    if (fleetsing_pointer_page_is_active()) {
        fleetsing_render_pointer_panel_remote();
        return;
    }

    if (fleetsing_numword_page_is_active() && fleetsing_numword_page_on_this_half()) {
        fleetsing_render_numword_panel();
        return;
    }

    fleetsing_render_top_padding(FLEETSING_OLED_OFFHAND_TOP_PAD);

    if (fleetsing_format_alert_state(fleetsing_display_remote_state.locked_layers_mask, fleetsing_display_remote_state.oneshot_mods, fleetsing_display_remote_state.oneshot_locked_mods, fleetsing_display_remote_state.host_leds, value, sizeof(value))) {
        fleetsing_render_pair("Alert", value);
        fleetsing_format_os_mode_state(fleetsing_display_remote_state.os_mode, value, sizeof(value));
        fleetsing_render_pair("OS", value);
        fleetsing_format_host_leds_state(fleetsing_display_remote_state.host_leds, value, sizeof(value));
        fleetsing_render_pair("Host", value);
        return;
    }

    fleetsing_format_os_mode_state(fleetsing_display_remote_state.os_mode, value, sizeof(value));
    fleetsing_render_pair("OS", value);
    fleetsing_format_macro_status_state(fleetsing_display_remote_state.macro_status, value, sizeof(value));
    fleetsing_render_pair("Macro", value);
    fleetsing_format_host_leds_state(fleetsing_display_remote_state.host_leds, value, sizeof(value));
    fleetsing_render_pair("Host", value);
#endif
}

#ifdef SPLIT_TRANSACTION_IDS_USER
static uint8_t fleetsing_host_led_state_bits(void) {
    led_t led_state = host_keyboard_led_state();
    return (led_state.num_lock ? 0x01 : 0) | (led_state.caps_lock ? 0x02 : 0) | (led_state.scroll_lock ? 0x04 : 0);
}

static void fleetsing_fill_display_sync_state(fleetsing_display_sync_t *state) {
    state->layer               = get_highest_layer(layer_state);
    state->locked_layers_mask  = fleetsing_locked_layers_mask();
    state->pointer_flags       = fleetsing_pointer_flags_state();
    state->dpi                 = fleetsing_get_active_pointer_dpi();
    state->os_mode             = (uint8_t)fleetsing_get_os_mode();
    state->macro_status        = (uint8_t)fleetsing_macro_status;
    state->host_leds           = fleetsing_host_led_state_bits();
    state->oneshot_mods        = get_oneshot_mods();
    state->oneshot_locked_mods = get_oneshot_locked_mods();
    state->overlay_active      = fleetsing_local_overlay_is_active();
    state->overlay_count       = fleetsing_oled_overlay_state.count;

    for (uint8_t i = 0; i < FLEETSING_OLED_OVERLAY_MAX_ITEMS; ++i) {
        if (i < fleetsing_oled_overlay_state.count) {
            memcpy(&state->overlay_items[i], &fleetsing_oled_overlay_state.items[i], sizeof(state->overlay_items[i]));
        } else {
            memset(&state->overlay_items[i], 0, sizeof(state->overlay_items[i]));
        }
    }
}

static void fleetsing_display_sync_handler(uint8_t initiator2target_buffer_size, const void *initiator2target_buffer, uint8_t target2initiator_buffer_size, void *target2initiator_buffer) {
    (void)target2initiator_buffer_size;
    (void)target2initiator_buffer;

    if (initiator2target_buffer_size == sizeof(fleetsing_display_remote_state)) {
        memcpy(&fleetsing_display_remote_state, initiator2target_buffer, sizeof(fleetsing_display_remote_state));
    }
}

void fleetsing_display_post_init(void) {
    transaction_register_rpc(RPC_ID_USER_DISPLAY_SYNC, fleetsing_display_sync_handler);
}

void fleetsing_display_sync_task(void) {
    if (!is_keyboard_master()) {
        return;
    }

    static fleetsing_display_sync_t last_sent_state = {0};
    static uint32_t                 last_sync       = 0;
    fleetsing_display_sync_t        current_state   = {0};

    fleetsing_fill_display_sync_state(&current_state);

    if (!memcmp(&current_state, &last_sent_state, sizeof(current_state)) && timer_elapsed32(last_sync) <= 500) {
        return;
    }

    if (transaction_rpc_send(RPC_ID_USER_DISPLAY_SYNC, sizeof(current_state), &current_state)) {
        memcpy(&last_sent_state, &current_state, sizeof(current_state));
        last_sync = timer_read32();
    }
}
#else
void fleetsing_display_post_init(void) {}

void fleetsing_display_sync_task(void) {}
#endif

oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    (void)rotation;
    return is_keyboard_left() ? FLEETSING_OLED_ROTATION_LEFT : FLEETSING_OLED_ROTATION_RIGHT;
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
    bool left_half = is_keyboard_left();

    fleetsing_update_oled_overlay();

    if (is_keyboard_master()) {
        /*
         * Use the userspace-owned key-activity timer for OLED sleep/wake so
         * split trackball chatter cannot keep the displays alive forever.
         * The USB half owns the power state and split OLED sync mirrors it
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

    if (!left_half && fleetsing_overlay_is_active()) {
        size_t  offset = snprintf(snapshot, sizeof(snapshot), "X");
        uint8_t count  = is_keyboard_master() ? fleetsing_oled_overlay_state.count : fleetsing_display_remote_state.overlay_count;

        for (uint8_t i = 0; i < count && offset < sizeof(snapshot); ++i) {
            const fleetsing_oled_overlay_item_t *item = is_keyboard_master() ? &fleetsing_oled_overlay_state.items[i] : &fleetsing_display_remote_state.overlay_items[i];
            offset += snprintf(snapshot + offset, sizeof(snapshot) - offset, "|%s|%s", item->title, item->value);
        }

        if (!fleetsing_update_oled_snapshot(snapshot)) {
            return false;
        }

        oled_clear();
        oled_set_cursor(0, 0);
        if (is_keyboard_master()) {
            fleetsing_render_overlay();
        } else {
            fleetsing_render_overlay_remote();
        }
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
    } else if (left_half) {
        char field_a[FLEETSING_OLED_VALUE_SIZE];
        char field_b[FLEETSING_OLED_VALUE_SIZE];
        char field_c[FLEETSING_OLED_VALUE_SIZE];

        if (is_keyboard_master()) {
            fleetsing_format_locked_layers(field_a, sizeof(field_a));
            fleetsing_format_pointer_mode(field_b, sizeof(field_b));
            snprintf(field_c, sizeof(field_c), "%u", fleetsing_get_active_pointer_dpi());
            snprintf(snapshot, sizeof(snapshot), "L|%s|%s|%s|%s", fleetsing_layer_name(get_highest_layer(layer_state)), field_a, field_b, field_c);
        } else {
            fleetsing_format_locked_layers_state(fleetsing_display_remote_state.locked_layers_mask, field_a, sizeof(field_a));
            fleetsing_format_pointer_mode_state(fleetsing_display_remote_state.pointer_flags, field_b, sizeof(field_b));
            snprintf(field_c, sizeof(field_c), "%u", fleetsing_display_remote_state.dpi);
            snprintf(snapshot, sizeof(snapshot), "L|%s|%s|%s|%s", fleetsing_layer_name(fleetsing_display_remote_state.layer), field_a, field_b, field_c);
        }

        if (!fleetsing_update_oled_snapshot(snapshot)) {
            return false;
        }

        oled_clear();
        oled_set_cursor(0, 0);
        if (is_keyboard_master()) {
            fleetsing_render_master_panel();
        } else {
            fleetsing_render_left_panel_remote();
        }
    } else {
        char alert[FLEETSING_OLED_VALUE_SIZE];
        char os[FLEETSING_OLED_VALUE_SIZE];
        char field_c[FLEETSING_OLED_VALUE_SIZE];
        char host[FLEETSING_OLED_VALUE_SIZE];
        bool has_alert = is_keyboard_master() ? fleetsing_format_alert(alert, sizeof(alert)) : fleetsing_format_alert_state(fleetsing_display_remote_state.locked_layers_mask, fleetsing_display_remote_state.oneshot_mods, fleetsing_display_remote_state.oneshot_locked_mods, fleetsing_display_remote_state.host_leds, alert, sizeof(alert));

        if (fleetsing_macro_page_is_active()) {
            if (is_keyboard_master()) {
                fleetsing_format_macro_status(alert, sizeof(alert));
                fleetsing_format_macro_page_action(os, sizeof(os));
                fleetsing_format_macro_page_slot(field_c, sizeof(field_c));
            } else {
                fleetsing_format_macro_status_state(fleetsing_display_remote_state.macro_status, alert, sizeof(alert));
                fleetsing_format_macro_page_action_state(fleetsing_display_remote_state.macro_status, os, sizeof(os));
                fleetsing_format_macro_page_slot_state(fleetsing_display_remote_state.macro_status, field_c, sizeof(field_c));
            }
            snprintf(snapshot, sizeof(snapshot), "R|M|%s|%s|%s", alert, os, field_c);
        } else if (fleetsing_pointer_page_is_active()) {
            if (is_keyboard_master()) {
                snprintf(snapshot, sizeof(snapshot), "R|P|%s|%s|%u", fleetsing_toggle_name(charybdis_get_pointer_sniping_enabled()), fleetsing_toggle_name(charybdis_get_pointer_dragscroll_enabled()), fleetsing_get_active_pointer_dpi());
            } else {
                snprintf(snapshot, sizeof(snapshot), "R|P|%s|%s|%u", fleetsing_toggle_name((fleetsing_display_remote_state.pointer_flags & ((uint8_t)1 << 0)) != 0), fleetsing_toggle_name((fleetsing_display_remote_state.pointer_flags & ((uint8_t)1 << 1)) != 0), fleetsing_display_remote_state.dpi);
            }
        } else if (fleetsing_numword_page_is_active() && fleetsing_numword_page_on_this_half()) {
            fleetsing_format_numword_remaining(field_c, sizeof(field_c));
            snprintf(snapshot, sizeof(snapshot), "R|W|%s", field_c);
        } else if (has_alert) {
            if (is_keyboard_master()) {
                fleetsing_format_os_mode(field_c, sizeof(field_c));
                fleetsing_format_host_leds(host, sizeof(host));
            } else {
                fleetsing_format_os_mode_state(fleetsing_display_remote_state.os_mode, field_c, sizeof(field_c));
                fleetsing_format_host_leds_state(fleetsing_display_remote_state.host_leds, host, sizeof(host));
            }
            snprintf(snapshot, sizeof(snapshot), "R|A|%s|%s|%s", alert, field_c, host);
        } else {
            if (is_keyboard_master()) {
                fleetsing_format_os_mode(os, sizeof(os));
                fleetsing_format_macro_status(field_c, sizeof(field_c));
                fleetsing_format_host_leds(host, sizeof(host));
            } else {
                fleetsing_format_os_mode_state(fleetsing_display_remote_state.os_mode, os, sizeof(os));
                fleetsing_format_macro_status_state(fleetsing_display_remote_state.macro_status, field_c, sizeof(field_c));
                fleetsing_format_host_leds_state(fleetsing_display_remote_state.host_leds, host, sizeof(host));
            }
            snprintf(snapshot, sizeof(snapshot), "R|N|%s|%s|%s", os, field_c, host);
        }

        if (!fleetsing_update_oled_snapshot(snapshot)) {
            return false;
        }

        oled_clear();
        oled_set_cursor(0, 0);
        if (is_keyboard_master()) {
            fleetsing_render_offhand_panel();
        } else {
            fleetsing_render_right_panel_remote();
        }
    }

    return false;
}
#endif

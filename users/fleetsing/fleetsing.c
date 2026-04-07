#include "fleetsing.h"

static bool fleetsing_scrolling_enabled = false;

void fleetsing_set_scrolling_enabled(bool enabled) {
    fleetsing_scrolling_enabled = enabled;
}

bool fleetsing_get_scrolling_enabled(void) {
    return fleetsing_scrolling_enabled;
}
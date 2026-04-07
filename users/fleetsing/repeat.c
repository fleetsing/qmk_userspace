#include "fleetsing.h"

bool remember_last_key_user(uint16_t keycode, keyrecord_t *record, uint8_t *remembered_mods) {
    if (keycode == QK_REP || keycode == QK_AREP) {
        return false;
    }

    return true;
}

bool fleetsing_repeat_process_record(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case QK_AREP:
            if (record->tap.count) {
                alt_repeat_key_invoke(&record->event);
                return false;
            }
            return true;
        case QK_REP:
            if (record->tap.count) {
                repeat_key_invoke(&record->event);
                return false;
            }
            return true;
        default:
            return true;
    }
}

#include "fleetsing.h"
#include "layouts/charybdis_3x5/layout_positions.h"

bool get_custom_auto_shifted_key(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case S(FI_4):
        case A(FI_2):
        case FI_DOT:
        case FI_COMM:
        case FI_PLUS:
        case FI_MINS:
        case S(FI_7):
        case S(FI_QUOT):
        case S(FI_DOT):
        case A(FI_DIAE):
        case S(FI_8):
        case A(FI_8):
        case S(FI_9):
        case A(FI_9):
        case FI_QUOT:
        case S(FI_6):
        case _L25:
        case _L24:
        case _L23:
        case _L22:
        case _L32:
        case _L33:
        case _R35:
        case _R33:
        case _R32:
        case _R22:
        case _R23:
        case _R24:
        case _R25:
            return true;
        default:
            return false;
    }
}

void autoshift_press_user(uint16_t keycode, bool shifted, keyrecord_t *record) {
    switch (keycode) {
        case S(FI_4):
            register_code16((!shifted) ? S(FI_4) : A(FI_4));
            break;
        case A(FI_2):
            register_code16((!shifted) ? A(FI_2) : S(FI_DIAE));
            break;
        case FI_DOT:
            register_code16((!shifted) ? FI_DOT : S(FI_PLUS));
            break;
        case FI_COMM:
            register_code16((!shifted) ? FI_COMM : S(FI_1));
            break;
        case FI_PLUS:
            register_code16((!shifted) ? FI_PLUS : S(FI_0));
            break;
        case FI_MINS:
            register_code16((!shifted) ? FI_MINS : S(FI_MINS));
            break;
        case S(FI_7):
            register_code16((!shifted) ? S(FI_7) : S(A(FI_7)));
            break;
        case S(FI_QUOT):
            register_code16((!shifted) ? S(FI_QUOT) : S(FI_3));
            break;
        case S(FI_DOT):
            register_code16((!shifted) ? S(FI_DOT) : S(FI_COMM));
            break;
        case A(FI_DIAE):
            register_code16((!shifted) ? A(FI_DIAE) : A(FI_7));
            break;
        case S(FI_8):
            register_code16((!shifted) ? S(FI_8) : S(A(FI_8)));
            break;
        case A(FI_8):
            register_code16((!shifted) ? A(FI_8) : FI_SECT);
            break;
        case S(FI_9):
            register_code16((!shifted) ? S(FI_9) : S(A(FI_9)));
            break;
        case A(FI_9):
            register_code16((!shifted) ? A(FI_9) : S(FI_SECT));
            break;
        case FI_QUOT:
            register_code16((!shifted) ? FI_QUOT : S(FI_2));
            break;
        case S(FI_6):
            register_code16((!shifted) ? S(FI_6) : S(FI_5));
            break;
        default:
            if (shifted) {
                add_weak_mods(MOD_BIT(KC_LSFT));
            }
            register_code16((IS_RETRO(keycode)) ? keycode & 0xFF : keycode);
    }
}

void autoshift_release_user(uint16_t keycode, bool shifted, keyrecord_t *record) {
    switch (keycode) {
        case S(FI_4):
            unregister_code16((!shifted) ? S(FI_4) : A(FI_4));
            if (shifted) {
                set_last_keycode(A(FI_4));
            }
            break;
        case A(FI_2):
            unregister_code16((!shifted) ? A(FI_2) : S(FI_DIAE));
            if (shifted) {
                set_last_keycode(S(FI_DIAE));
            }
            break;
        case FI_DOT:
            unregister_code16((!shifted) ? FI_DOT : S(FI_PLUS));
            if (shifted) {
                set_last_keycode(S(FI_PLUS));
            }
            break;
        case FI_COMM:
            unregister_code16((!shifted) ? FI_COMM : S(FI_1));
            if (shifted) {
                set_last_keycode(S(FI_1));
            }
            break;
        case FI_PLUS:
            unregister_code16((!shifted) ? FI_PLUS : S(FI_0));
            if (shifted) {
                set_last_keycode(S(FI_0));
            }
            break;
        case FI_MINS:
            unregister_code16((!shifted) ? FI_MINS : S(FI_MINS));
            if (shifted) {
                set_last_keycode(S(FI_MINS));
            }
            break;
        case S(FI_7):
            unregister_code16((!shifted) ? S(FI_7) : S(A(FI_7)));
            if (shifted) {
                set_last_keycode(S(A(FI_7)));
            }
            break;
        case S(FI_QUOT):
            unregister_code16((!shifted) ? S(FI_QUOT) : S(FI_3));
            if (shifted) {
                set_last_keycode(S(FI_3));
            }
            break;
        case S(FI_DOT):
            unregister_code16((!shifted) ? S(FI_DOT) : S(FI_COMM));
            if (shifted) {
                set_last_keycode(S(FI_COMM));
            }
            break;
        case A(FI_DIAE):
            unregister_code16((!shifted) ? A(FI_DIAE) : A(FI_7));
            if (shifted) {
                set_last_keycode(A(FI_7));
            }
            break;
        case S(FI_8):
            unregister_code16((!shifted) ? S(FI_8) : S(A(FI_8)));
            if (shifted) {
                set_last_keycode(S(A(FI_8)));
            }
            break;
        case A(FI_8):
            unregister_code16((!shifted) ? A(FI_8) : FI_SECT);
            if (shifted) {
                set_last_keycode(FI_SECT);
            }
            break;
        case S(FI_9):
            unregister_code16((!shifted) ? S(FI_9) : S(A(FI_9)));
            if (shifted) {
                set_last_keycode(S(A(FI_9)));
            }
            break;
        case A(FI_9):
            unregister_code16((!shifted) ? A(FI_9) : S(FI_SECT));
            if (shifted) {
                set_last_keycode(S(FI_SECT));
            }
            break;
        case FI_QUOT:
            unregister_code16((!shifted) ? FI_QUOT : S(FI_2));
            if (shifted) {
                set_last_keycode(S(FI_2));
            }
            break;
        case S(FI_6):
            unregister_code16((!shifted) ? S(FI_6) : S(FI_5));
            if (shifted) {
                set_last_keycode(S(FI_5));
            }
            break;
        default:
            unregister_code16((IS_RETRO(keycode)) ? keycode & 0xFF : keycode);
    }
}

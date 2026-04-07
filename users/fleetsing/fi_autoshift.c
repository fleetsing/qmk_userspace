#include "fleetsing.h"

bool get_custom_auto_shifted_key(uint16_t keycode, keyrecord_t *record) {
    switch(keycode) {
        case S(FI_4):
            return true;
        case A(FI_2):
            return true;
        case FI_DOT:
            return true;
        case FI_COMM:
            return true;
        case FI_PLUS:
            return true;
        case FI_MINS:
            return true;
        case S(FI_7):
            return true;
        case S(FI_QUOT):
            return true;
        case S(FI_DOT):
            return true;
        case A(FI_DIAE):
            return true;
        case S(FI_8):
            return true;
        case A(FI_8):
            return true;
        case S(FI_9):
            return true;
        case A(FI_9):
            return true;
        case FI_QUOT:
            return true;
        case S(FI_6):
            return true;
        case _L25:
            return true;
        case _L24:
            return true;
        case _L23:
            return true;
        case _L22:
            return true;
        case _L32:
            return true;
        case _L33:
            return true;
        case _R35:
            return true;
        case _R33:
            return true;
        case _R32:
            return true;
        case _R22:
            return true;
        case _R23:
            return true;
        case _R24:
            return true;
        case _R25:
            return true;
        default:
            return false;
    }
}

void autoshift_press_user(uint16_t keycode, bool shifted, keyrecord_t *record) {
    switch(keycode) {
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
            // & 0xFF gets the Tap key for Tap Holds, required when using Retro Shift
            register_code16((IS_RETRO(keycode)) ? keycode & 0xFF : keycode);
    }
}

void autoshift_release_user(uint16_t keycode, bool shifted, keyrecord_t *record) {
    switch(keycode) {
        case S(FI_4):
            unregister_code16((!shifted) ? S(FI_4) : A(FI_4));
            if (shifted) { set_last_keycode(A(FI_4)); }
            break;
        case A(FI_2):
            unregister_code16((!shifted) ? A(FI_2) : S(FI_DIAE));
            if (shifted) { set_last_keycode(S(FI_DIAE)); }
            break;
        case FI_DOT:
            unregister_code16((!shifted) ? FI_DOT : S(FI_PLUS));
            if (shifted) { set_last_keycode(S(FI_PLUS)); }
            break;
        case FI_COMM:
            unregister_code16((!shifted) ? FI_COMM : S(FI_1));
            if (shifted) { set_last_keycode(S(FI_1)); }
            break;
        case FI_PLUS:
            unregister_code16((!shifted) ? FI_PLUS : S(FI_0));
            if (shifted) { set_last_keycode(S(FI_0)); }
            break;
        case FI_MINS:
            unregister_code16((!shifted) ? FI_MINS : S(FI_MINS));
            if (shifted) { set_last_keycode(S(FI_MINS)); }
            break;
        case S(FI_7):
            unregister_code16((!shifted) ? S(FI_7) : S(A(FI_7)));
            if (shifted) { set_last_keycode(S(A(FI_7))); }
            break;
        case S(FI_QUOT):
            unregister_code16((!shifted) ? S(FI_QUOT) : S(FI_3));
            if (shifted) { set_last_keycode(S(FI_3)); }
            break;
        case S(FI_DOT):
            unregister_code16((!shifted) ? S(FI_DOT) : S(FI_COMM));
            if (shifted) { set_last_keycode(S(FI_COMM)); }
            break;
        case A(FI_DIAE):
            unregister_code16((!shifted) ? A(FI_DIAE) : A(FI_7));
            if (shifted) { set_last_keycode(A(FI_7)); }
            break;
        case S(FI_8):
            unregister_code16((!shifted) ? S(FI_8) : S(A(FI_8)));
            if (shifted) { set_last_keycode(S(A(FI_8))); }
            break;
        case A(FI_8):
            unregister_code16((!shifted) ? A(FI_8) : FI_SECT);
            if (shifted) { set_last_keycode(FI_SECT); }
            break;
        case S(FI_9):
            unregister_code16((!shifted) ? S(FI_9) : S(A(FI_9)));
            if (shifted) { set_last_keycode(S(A(FI_9))); }
            break;
        case A(FI_9):
            unregister_code16((!shifted) ? A(FI_9) : S(FI_SECT));
            if (shifted) { set_last_keycode(S(FI_SECT)); }
            break;
        case FI_QUOT:
            unregister_code16((!shifted) ? FI_QUOT : S(FI_2));
            if (shifted) { set_last_keycode(S(FI_2)); }
            break;
        case S(FI_6):
            unregister_code16((!shifted) ? S(FI_6) : S(FI_5));
            if (shifted) { set_last_keycode(S(FI_5)); }
            break;
        default:
            // & 0xFF gets the Tap key for Tap Holds, required when using Retro Shift
            // The IS_RETRO check isn't really necessary here, always using
            // keycode & 0xFF would be fine.
            unregister_code16((IS_RETRO(keycode)) ? keycode & 0xFF : keycode);
    }
    // Remember that the key was autoshifted.
}

uint16_t get_alt_repeat_key_keycode_user(uint16_t keycode, uint8_t mods) {
    switch (keycode) {
        case KC_TAB: return S(KC_TAB);  // Tab reverses to Shift + Tab.
        case S(KC_TAB): return KC_TAB;  // Shift + Tab reverses to Tab.
        
        case G(FI_Y): return G(FI_Z);  // GUI + Y reverses to GUI + Z.
        case G(FI_Z): return G(FI_Y);  // GUI + Z reverses to GUI + Y.
        case G(FI_C): return G(FI_V);  // GUI + C reverses to GUI + V.

        case C(FI_Y): return C(FI_Z);  // Ctrl + Y reverses to Ctrl + Z.
        case C(FI_Z): return C(FI_Y);  // Ctrl + Z reverses to Ctrl + Y.
        case C(FI_C): return C(FI_V);  // Ctrl + C reverses to Ctrl + V.
    
        case FI_LPRN: return FI_RPRN;  // Left Parenthesis reverses to Right Parenthesis.
        case A(FI_8): return A(FI_9);  // Left Brakcet reverses to Right Bracket.
        case A(FI_LPRN): return A(FI_RPRN);  // Left Brace reverses to Right Brace.

        case FI_SECT: return FI_HALF;  // Less Than reverses to Greater Than.
        case FI_HALF: return FI_SECT;  // Greater Than reverses to Less Than.
    }
    return KC_TRNS;
}
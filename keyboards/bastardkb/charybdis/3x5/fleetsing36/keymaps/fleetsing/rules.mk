# Load the shared fleetsing userspace under users/fleetsing/.
USER_NAME := fleetsing
# VIA is intentionally disabled; this keymap is source-controlled rather than GUI-configured.
VIA_ENABLE = no
# Charybdis uses PMW3360 sensors for the trackballs.
POINTING_DEVICE_DRIVER = pmw3360
# Enables MS_BTN* keycodes and the mousekey helpers used on the pointer layer.
MOUSEKEY_ENABLE = yes
# Required by the Finnish Auto Shift overrides in users/fleetsing/layouts/charybdis_3x5/.
AUTO_SHIFT_ENABLE = yes
# Enables QK_REP and QK_AREP used by the thumb combos.
REPEAT_KEY_ENABLE = yes
# Enables QK_LLCK on the numbers and navigation layers.
LAYER_LOCK_ENABLE = yes
# Enables the positional combos defined in layouts/charybdis_3x5/combos.def.
COMBO_ENABLE = yes
# Enables DM_REC*, DM_PLY*, and DM_RSTP on the macro layer.
DYNAMIC_MACRO_ENABLE = yes
# OLED status display on both halves.
OLED_ENABLE = yes
OLED_DRIVER = ssd1306
OLED_TRANSPORT = i2c
# DRV2605L haptic driver for the installed actuator.
HAPTIC_ENABLE = yes
HAPTIC_DRIVER = drv2605l

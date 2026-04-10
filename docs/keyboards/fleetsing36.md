# Fleetsing36

`fleetsing36` is the active custom Charybdis Nano 36-key target in this
userspace:

- Keyboard: `bastardkb/charybdis/3x5/fleetsing36`
- Keymap: `fleetsing`

This page is the workspace-specific description for the board. It is the right
place to grow more visual and personal over time with photos, build notes, and
layout explanations.

## Physical Setup

Current board-level facts from the checked-out workspace:

- Split Charybdis Nano 3x5 layout with 36 total keys
- Dual pointing sensors with combined pointing enabled in userspace
- RP2040 converter target via `CONVERT_TO = rp2040_ce`
- `EE_HANDS` enabled so left/right identity is stored per half
- SH1107 64x128 OLED modules
- DRV2605L haptic driver

The custom board definition lives in:

- `qmk_firmware/keyboards/bastardkb/charybdis/3x5/fleetsing36/`

## Layout And Behavior Highlights

The active userspace is built around a compact typing layout with a few strong
themes:

- Heavy use of home-row and thumb combos for punctuation, repeat, and mode
  switching
- A temporary `NumWord` layer for short numeric bursts
- A dedicated numbers layer for denser number entry
- Separate navigation, function, media, pointer, and macro layers
- OS mode switching for macOS vs PC modifier behavior
- Userspace-managed scroll-side selection while Charybdis firmware remains the
  source of truth for DPI and sniping DPI
- Layout-specific Finnish Auto Shift behavior

The main implementation lives in:

- `keyboards/bastardkb/charybdis/3x5/fleetsing36/keymaps/fleetsing/keymap.c`
- `users/fleetsing/fleetsing.c`
- `users/fleetsing/pointing.c`
- `users/fleetsing/layouts/charybdis_3x5/combos.def`
- `users/fleetsing/layouts/charybdis_3x5/fi_autoshift.c`

## Planned Additions

This page is intended to grow. Good future additions:

- photos of the board
- a labeled layer diagram
- notes on physical build choices
- a summary of each layer's purpose
- a short explanation of combo families and pointer-mode behavior

## Verification

Primary compile command:

`qmk compile -kb bastardkb/charybdis/3x5/fleetsing36 -km fleetsing`

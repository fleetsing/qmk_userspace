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

## Layer Summary

- Base: alpha layout plus home-row mods, thumb layer-taps, and the shared
  physical positions that combos build on.
- NumWord: temporary numeric burst layer entered by the `L43 + R43` thumb
  combo. It stays alive for digits, common numeric symbols, editing keys, and
  cursor movement so inline number editing does not immediately cancel it.
- Numbers: denser numeric entry with symbols and editing helpers on the left
  hand and the digit cluster on the right.
- Navigation: cursor movement and document navigation, with the right thumb as
  the hold key that exposes it from base.
- Function: F-keys and function-row modifiers.
- Symbols: dedicated coding punctuation that avoids overloading combo space for
  every programming symbol.
- Media: OS mode toggles and media/system navigation. It is reached as a
  tri-layer by holding Numbers and Navigation together, and it deliberately has
  no direct bootloader key.
- Pointer: pointer-layer controls, drag-scroll, scroll-side selection, and the
  guarded `BOOT_SAFE` hold for flashing.
- Macro: dynamic macro recording/playback and common desktop clipboard chords.

## Combo Highlights

- `L43 + R43`: `NumWord`
- `L43 + L41`: `QK_REP`
- `R41 + R43`: `QK_AREP`
- `L41 + R41`: `Caps Word`
- Vertical same-finger combos on the alpha columns: high-frequency symbol
  access with Finnish-specific Auto Shift behavior

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
- a visual map of the layer and combo families summarized above
- a short explanation of pointer-mode behavior and scroll-side selection

## Verification

Primary compile command:

`qmk compile -kb bastardkb/charybdis/3x5/fleetsing36 -km fleetsing`

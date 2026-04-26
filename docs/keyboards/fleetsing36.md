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
- Right-side trackball with a left rotary encoder in place of the former left
  trackball
- RP2040 converter target via `CONVERT_TO = rp2040_ce`
- `EE_HANDS` enabled so left/right identity is stored per half
- Replacement SSD1312 128x64 OLED modules, with the original SH1107 64x128
  modules documented as the older known-good profile
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
- Transparent left and right scroll-hold layers that turn the trackball into
  drag-scroll while either middle-finger bottom-row key is held
- A left encoder that uses volume on Base and Media, arrows on Navigation, and
  scroll-wheel output elsewhere
- OS mode switching for macOS vs PC modifier behavior
- Userspace-managed drag-scroll activation while Charybdis firmware remains
  the source of truth for DPI and sniping DPI
- Layout-specific Finnish Auto Shift behavior

## Layer Summary

- Base: alpha layout plus home-row mods, thumb layer-taps, Hyper/Meh holds on
  the top row, and the shared physical positions that combos build on. `Q` and
  `Z` are swapped from the earlier base layout, and comma/dot are swapped
  together with their Finnish Auto Shift outputs.
- NumWord: temporary numeric burst layer entered by the `L43 + R43` thumb
  combo. It stays alive for digits, common numeric symbols, editing keys, and
  cursor movement so inline number editing does not immediately cancel it.
- Numbers: denser numeric entry with symbols and editing helpers on the left
  hand and the digit cluster on the right. It is held from `L41`.
- Navigation: cursor movement and document navigation. It is held from either
  `L43` or `R43`; holding both navigation thumbs promotes to Symbols.
- Function: F-keys and function-row modifiers. It is held from `R42`.
- Symbols: dedicated coding punctuation that avoids overloading combo space for
  every programming symbol. It is reached by holding both navigation thumbs
  `L43 + R43`.
- Media: OS mode toggles and media/system navigation. It is reached as a
  direct hold from `R41`, and it deliberately has no direct bootloader key.
- Pointer: pointer-layer controls, drag-scroll, sniping controls, and the
  guarded `BOOT_SAFE` hold for flashing.
- Macro: dynamic macro recording/playback and common desktop clipboard chords.
  It is reached by holding `L41 + R41`, the Numbers + Media tri-layer.
- Scroll Left / Scroll Right: transparent hold layers entered from `L33` and
  `R33`; either one enables Charybdis drag-scroll. Vertical drag-scroll is
  configured so physical trackball up scrolls up and physical down scrolls
  down.
- Encoder: Base and Media turns send volume up/down; Navigation sends down/up
  arrows; other layers send mouse-wheel scroll. The direct GPIO encoder press
  pin is initialized with an internal pull-up, but no key action is assigned
  while the hardware path is prone to phantom clicks. Encoder rotation actions
  suppress haptics.
- OLED: short-lived right-side change cards are synced from the master but time
  out locally, so the right OLED returns to its default/status view after the
  overlay expires. Visible OLED text is rendered with a one-character left
  inset so it does not sit directly on the display edge. Progress bars should
  bypass that inset and render flush-left unless they are intentionally shorter,
  because the inset can push full-width bars onto a second row.

## Combo Highlights

- `L43 + R43`: `NumWord`
- holding `L43 + R43`: Symbols
- `L43 + L41`: `QK_REP`
- `R41 + R43`: `QK_AREP`
- `L41 + R41`: `Caps Word`
- holding `L41 + R41`: Macro
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
- a visual explanation of pointer, scroll-hold, and encoder behavior

## Verification

Primary compile command:

`qmk compile -kb bastardkb/charybdis/3x5/fleetsing36 -km fleetsing`

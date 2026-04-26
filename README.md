# Fleetsing QMK Userspace

This repository is the active external userspace for a custom Charybdis Nano
36-key setup. It holds the behavior layer of the project: the keymap, shared
userspace code, layout-specific helpers, and the personal logic that makes the
board feel like this board instead of a stock QMK example.

It sits beside a sibling firmware repository at `../qmk_firmware/`, which owns
the custom board definition and lower-level hardware details.

If you want the full picture rather than only the userspace slice, the
companion repositories are:

- Workspace metadata:
  <https://github.com/fleetsing/qmk_workspace>
- Userspace:
  <https://github.com/fleetsing/qmk_userspace>
- Firmware:
  <https://github.com/fleetsing/qmk_firmware>

## What Lives Here

This userspace currently centers on:

- the active `bastardkb/charybdis/3x5/fleetsing36` keyboard target
- the `fleetsing` keymap
- shared userspace modules under `users/fleetsing/`
- layout-specific combo and Auto Shift logic under
  `users/fleetsing/layouts/charybdis_3x5/`

This is where the board's personality lives:

- layers and layer interactions
- combos and repeat-key usage
- NumWord behavior
- OLED status UI
- haptics
- pointer, drag-scroll, encoder, and userspace pointing behavior
- OS-mode switching and Finnish-layout symbol handling

## Behavior Notes

- `NumWord` is the quick numeric burst mode entered from the `L43 + R43` thumb
  combo. It stays active through digit entry, common numeric punctuation, edit
  keys, and cursor movement keys such as arrows, Home/End, and Page Up/Down.
- Thumb holds currently expose Navigation from `L43` and `R43`, Numbers from
  `L41`, Function from `R42`, and Media from `R41`.
- Holding both Navigation thumbs, `L43 + R43`, promotes to Symbols. Holding
  Numbers and Media, `L41 + R41`, promotes to Macro.
- Holding either bottom-row middle-finger key, `L33` or `R33`, turns the
  trackball into drag-scroll through transparent scroll layers. Vertical
  drag-scroll is configured so physical trackball up scrolls up and physical
  down scrolls down.
- The left encoder sends volume up/down on Base and Media, arrows on
  Navigation, and wheel scroll on other layers. Its direct GPIO press pin is
  initialized with an internal pull-up, but no key action is assigned while the
  hardware path is prone to phantom clicks.
- Bootloader entry is intentionally kept off the casual media layer. The only
  in-keymap boot access is the guarded `BOOT_SAFE` hold on the pointer layer.
- Both `OS_MAC` and `OS_PC` still assume a Finnish host keyboard layout. The OS
  mode changes modifier swapping and the platform-specific symbol chords, not
  the underlying language layout.

## Start Here

- Workspace overview: [`../README.md`](../README.md)
- Workspace policy and ownership rules: [`../AGENTS.md`](../AGENTS.md)
- Canonical workspace facts: [`../docs/qmk-context.yaml`](../docs/qmk-context.yaml)
- Board page: [`docs/keyboards/fleetsing36.md`](docs/keyboards/fleetsing36.md)

The rough split is:

- `qmk_workspace` for shared context, setup, and cross-repo documentation
- `qmk_userspace` for the active keymap and personal behavior
- `qmk_firmware` for board definition, hardware metadata, and low-level custom
  firmware behavior

## Active Build Target

- Keyboard: `bastardkb/charybdis/3x5/fleetsing36`
- Keymap: `fleetsing`

Build targets are tracked in `qmk.json`, which is the source of truth for this
userspace's active compile targets.

## Key Paths

- Keymap:
  `keyboards/bastardkb/charybdis/3x5/fleetsing36/keymaps/fleetsing/`
- Shared userspace:
  `users/fleetsing/`
- Layout-specific helpers:
  `users/fleetsing/layouts/charybdis_3x5/`

## Build And Verify

From the workspace root:

- Configure overlay:
  `qmk config user.overlay_dir="$(realpath qmk_userspace)"`
- Health check:
  `qmk userspace-doctor`
- Primary compile:
  `qmk compile -kb bastardkb/charybdis/3x5/fleetsing36 -km fleetsing`
- Compile the configured userspace targets:
  `qmk userspace-compile -c -p`

## Notes

- There could be historical `.hex` and `.uf2` files in the repo root that are
  generated artifacts. Do not treat them as the source of truth for active targets.
- The firmware/userspace split is intentional. Hardware and board-conversion
  details belong in `../qmk_firmware/`; personal behavior belongs here.
- This README is intentionally the front door, not the whole manual. As the
  workspace grows, keyboard-specific pages under `docs/` can carry richer
  descriptions, layer notes, photos, and diagrams.

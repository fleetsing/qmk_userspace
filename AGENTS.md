# qmk_userspace instructions

This repository is the active external userspace for the workspace. The parent workspace `../` contains shared Codex instructions, the canonical QMK skill, and `docs/qmk-context.yaml`. The sibling repository `../qmk_firmware/` contains the custom keyboard definition.

For Git-sensitive work, prefer launching Codex in this repository and granting the parent workspace separately:
`codex -C ~/qmk/qmk_userspace --add-dir ~/qmk`

When the parent workspace is available, read `../AGENTS.md` and `../docs/qmk-context.yaml` before making changes.

## Active source locations

- Keymap: `keyboards/bastardkb/charybdis/3x5/fleetsing36/keymaps/fleetsing/`
- Shared user code: `users/fleetsing/`
- Build targets: `qmk.json`

Generated `.hex` and `.uf2` files in the repository root are build artifacts. Do not edit them unless explicitly asked.

## Important workspace-specific gotchas

- `users/fleetsing/rules.mk` currently compiles `fleetsing.c`, `pointing.c`, `display.c`, and `haptics.c`.
- `users/fleetsing/fi_autoshift.c` exists but is not currently compiled.
- Auto Shift logic currently lives inside the active `keymap.c`, so editing `fi_autoshift.c` alone does not change behavior.

## When edits belong here

Use this repository for:

- layers and layout behavior
- custom keycodes and shared enums
- combos, tap dance, repeat key behavior, caps word behavior
- OLED, pointing, haptics, and other personal feature logic
- keymap-level `config.h` and `rules.mk`
- reusable logic in `users/fleetsing/`

If the task is about pins, matrix wiring, split transport, development-board settings, or board conversion, inspect `../qmk_firmware/` and consider moving the change there instead.

## Verification

Preferred verification commands:

- `qmk userspace-doctor`
- `qmk compile -kb bastardkb/charybdis/3x5/fleetsing36 -km fleetsing`
- `qmk userspace-compile -c -p` for broader shared changes

Keep edits minimal and preserve the current split between keymap logic and shared user logic unless the task is explicitly a cleanup or migration.

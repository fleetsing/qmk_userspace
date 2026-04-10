# qmk_userspace instructions

This repository is the active external userspace for the workspace. The parent workspace `../` is also its own git repository and contains shared Codex instructions, the canonical QMK skill, and `docs/qmk-context.yaml`. The sibling repository `../qmk_firmware/` contains the custom keyboard definition.

For Git-sensitive work, prefer launching Codex in this repository and granting the parent workspace separately:
`codex -C ~/qmk/qmk_userspace --add-dir ~/qmk`

When the parent workspace is available, read `../AGENTS.md` and `../docs/qmk-context.yaml` before making changes.
Treat `../docs/qmk-context.yaml` as authoritative for current paths, active targets, verification commands, and known gotchas. This file only adds userspace-local guidance.
Treat GitHub links in local README files as human-facing navigation only. If the corresponding repository or file exists locally in this workspace, prefer the local checkout over following remote links.

## Active Source Locations

For the exact current keymap path, shared user file inventory, layout-specific userspace modules, build targets, generated-artifact policy, and gotchas, read `../docs/qmk-context.yaml`.

## When edits belong here

Use this repository for:

- layers and layout behavior
- custom keycodes and shared enums
- combos, tap dance, repeat key behavior, caps word behavior
- OLED, pointing, haptics, and other personal feature logic
- keymap-level `config.h` and `rules.mk`
- reusable logic in `users/fleetsing/`
- layout-specific userspace helpers in `users/fleetsing/layouts/<layout-name>/`

Keep this split in mind:

- put generic userspace hooks and helpers in top-level `users/fleetsing/`
- put physical-layout-specific positional aliases, combos, and Auto Shift behavior under `users/fleetsing/layouts/`
- keep `keymap.c` focused on the actual layer matrix and any truly keymap-local glue

## Documentation Expectations

When changing behavior in this repository, keep nearby inline documentation up to date when it materially improves readability.

- prefer short block comments ahead of non-obvious helpers, state, and hook implementations
- update existing comments when behavior changes so code and documentation do not drift apart
- document screen/status abbreviations and other QMK-specific shorthand close to the code that renders or interprets them
- do not add boilerplate comments for self-evident lines; focus on intent, ownership, and behavior

## OLED UI Expectations

When extending the split OLED UI for this userspace:

- keep the master side anchored on stable, orientation-friendly context such as the current layer and primary status summary
- default temporary, modal, or drill-down pages to the non-master side unless there is a strong usability reason to interrupt the master-side overview
- if a temporary page or overlay is added, document which side owns it and why near the rendering code

If the task is about pins, matrix wiring, split transport, development-board settings, or board conversion, inspect `../qmk_firmware/` and consider moving the change there instead.

## Verification

Use the commands documented in `../docs/qmk-context.yaml`.
Keep edits minimal and preserve the current split between keymap logic and shared user logic unless the task is explicitly a cleanup or migration.

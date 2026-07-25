# Code organization and style

SFA and Dinosaur Planet are snapshots of the same evolving codebase. The
Dinosaur Planet decompilation is therefore a useful organization and naming
reference, but it is not source truth for SFA. Retail SFA evidence still wins
when it identifies a source file, symbol, type, or layout.

This document adopts the useful parts of Dinosaur Planet's structure without
forcing N64-specific boundaries onto the GameCube game.

## Header ownership

Headers are organized by what they describe, not by the binary that happens to
contain their code:

- `include/dolphin/`: Nintendo GameCube SDK and platform APIs.
- `include/sys/`: engine services such as objects, rendering, maps, memory, and
  resource loading.
- `include/game/`: game data, identities, and gameplay-facing types.
  Object records and placement/setup data live in `include/game/objects/`.
- `include/dlls/`: dynamically loaded module ABIs and shared descriptor types.
- `include/main/`: legacy staging area. Existing headers can move out
  incrementally; do not add a new header here when one of the semantic owners
  above is known.
- `include/track/`: the separate track executable until its recovered ownership
  supports a more specific split.

Implementation paths remain evidence-driven. Do not move a `src/main/*.c`
translation unit merely to mirror a header directory: retail strings, splits,
and recovered file boundaries take precedence over aesthetic symmetry.

## Public headers

Prefer one clear public owner header per subsystem. Split a header only when the
parts have genuinely different dependency or visibility requirements.

- Put the commonly used service API in the owner header, such as
  `sys/objects.h`. A narrower header such as `sys/objects/lifecycle.h` is
  appropriate when exposing those operations more broadly would create invalid
  handle conversions or unnecessary dependencies.
- Put record layouts and serialized/setup data in the game-owned header, such
  as `game/objects/object.h` and `game/objects/object_setup.h`.
- Use an `_internal.h` header only for definitions shared by several
  translation units of the same subsystem.
- Do not create a one-function `*_api.h` header just to remove a local `extern`.
  Add the declaration to the function's existing owner header.
- Avoid forwarding headers and compatibility aliases during a move. Update
  users in the same change so there is one canonical include path.
- Include what a header needs and keep unrelated implementation dependencies
  out of public headers.

Engine services and shared DLL ABIs may depend on stable game-owned records.
Gameplay and DLL implementations may depend on both. Platform headers should
not depend upward on any of them.

Recovered layouts can temporarily violate this direction. Record the debt and
fix it when the involved types are understood; do not hide a cycle behind more
facade headers.

## Naming

Known retail names take precedence. For names that must be inferred, follow the
Dinosaur Planet conventions:

- Core functions use a short subsystem prefix and `camelCase`, for example
  `objSetupObject` or `texLoadTexture`.
- DLL exports use `<prefix>_<PascalCase>`. Static DLL helpers use
  `<prefix>_<camelCase>`.
- Global variables use `gCamelCase`; translation-unit statics use
  `sCamelCase`; locals use `camelCase`.
- Types use `PascalCase`.
- Constants and macros use `SCREAMING_SNAKE_CASE`.

Do not churn an established, evidence-backed symbol only to satisfy the style.
Apply these rules when replacing an anonymous name or introducing a recovered
concept.

## Formatting

- Four spaces; never tabs.
- Opening braces stay on the declaration or control-statement line.
- Always brace control-flow bodies.
- Attach `*` to the type (`GameObject* obj`).
- Keep switch labels at the same indentation as `switch`.
- One statement per line.
- Use Unix line endings, remove trailing whitespace, and end files with a
  newline.

`.clang-format` encodes these defaults. Format files being actively cleaned up,
not the entire recovered tree in one sweep. Matching-sensitive source structure
still takes precedence over cosmetic formatting.

## Incremental migration

Move one coherent ownership cluster at a time:

1. Move or merge the owner headers and update every include in the same change.
2. Do not leave a second compatibility path behind.
3. Run `ninja all_source` to catch consumers outside the matching build.
4. Run the strict matching build to ensure a header-only move did not alter
   generated code.
5. Keep the change structural. Do not mix it with speculative type or function
   recovery.

The first migrated cluster establishes the pattern:

| Legacy path | Canonical owner |
|---|---|
| `include/main/game_object.h` | `include/game/objects/object.h` |
| `include/main/obj_placement.h` | `include/game/objects/object_setup.h` |
| `include/main/object_api.h` | `include/sys/objects.h` |
| `include/main/object.h` | `include/sys/objects/lifecycle.h` |
| `include/main/object_descriptor.h` | `include/dlls/object_descriptor.h` |

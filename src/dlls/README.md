# DLL source layout

This tree is the planned home for code currently under `src/main/dll/`.
Source files have not been moved yet: the empty leaf directories make the
intended DLL boundaries reviewable before existing translation units are
assigned to them.

Directory names use the SFA resource descriptor's flattened decimal slot:

```text
<bank>/<decimal slot>[_<known name>]/
```

The four banks and their boundaries come from SFA's extracted `DLLS.tab` and
the matching `gResourceDescriptors` array:

- `engine`: slots 0 through 88
- `modgfx`: slots 89 through 170
- `projgfx`: slots 171 through 194
- `objects`: slots 195 through 704

Slot 705 is the resource descriptor table's trailing sentinel, so it does not
have a source directory.

Names are included only where current SFA source, recovered DOL identifiers,
or retail `OBJECTS.bin` evidence supports them. Their capitalization and
underscores are retained exactly; they are not normalized for directory names.
A number-only directory means the name is not yet known well enough to put in
the path.

`dlls.txt` records the scaffold in a compact `slot = path` form. It is a
planning manifest for now and is not consumed by the build.

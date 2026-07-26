# SharpClaw / WarpStone conversion-pool boundary

`SCchieflightfoot` and WarpStone cannot be compiled as independent source
objects even though their functions are instruction-exact. Both use the same
signed-integer-to-double conversion constant at `0x803E5490`; they are now kept
together in `dlls/objects/432_SH_swapston/SH_swapston.c`.

## Retail evidence

- `SHthorntail_updateDustEffects` references the constant at `0x801D698C`,
  `0x801D6A04`, and `0x801D6AE8`.
- `warpstone_update` references it at `0x801D77CC` and `0x801D781C`.
- Retail contains one eight-byte object at `0x803E5490`, compiled in the shared
  `SH_swapston` TU as `@279_803E5490`.
- The surrounding retail text is ordered as the `SCchieflightfoot` region,
  `SClantern_advanceAnimEvents`, and then the WarpStone region. Debug-side source
  order further divides the WarpStone code between `SCcollectables.c` and
  `SCanimobj.c`. This is broader than a simple two-file ownership mistake.

## Source-link experiment

Compiling either current C file naturally emits a local eight-byte `@279`
pool. Promoting only WarpStone leaves the retail assembly object for
`scchieflightfoot` with an undefined reference to `@279_803E5490`. Promoting
both files links, but adds a second eight-byte pool: the DOL `.sdata2` size
increases from `0x28` to `0x30`, and every following small-data relocation is
shifted by eight bytes. The strict checksum therefore fails.

## Boundary conclusion

The retail constant belongs to a shared DLL-level compilation or sublink pool,
not independently to either helper region. The original DLL grouping is
therefore required and is represented by the single `SH_swapston.c` source. Do
not split the helper regions back into independent objects or solve the shared
pool with a symbol alias, forced section, padding, or a synthetic conversion
shim.

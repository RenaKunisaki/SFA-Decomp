# Unused Things

> Source: [Rena Kunisaki's SFA wiki](https://github.com/RenaKunisaki/StarFoxAdventures/wiki/UnusedThings). Reverse-engineering notes; not independently verified here.

Grab-bag of unused/cut/bugged content the wiki has documented: dead disc files, unused objects,
old asset versions, and some AR/Gecko debug codes. Several of the files named here (`GAMETEXT.bin`,
`SCREENS.bin`, `VOXOBJ.bin`, `DLLS.tab`, etc.) already have a much deeper fileId-level writeup in
[Files](Files) — this page links to that instead of repeating it, and focuses on what's specific to
*this* page: the unused-object/GameBit reports, the old-asset hex dumps, and the debug codes.

## Unused files (disc root)

- `/CACHEFONTSTAB.bin`, `/CACHEFONTSTEX.bin`: both 0 bytes.
- `/DLLS.tab`: presumably for an old (N64-era) DLL system.
- `/VOXOBJ.bin`, `/VOXOBJ.tab`: both 0 bytes, presumably related to `[map]/VOXMAP.[bin,tab]`.
- `/splashScreen.bin`: unused splash screen image.
- `/dfalls`, `/frontend`, `/swapcircle`: unused maps.
- `/modules`: two unused files, assumed to be part of Dolphin OS' module system that the game
  doesn't actually use.
- `/musyxbin`: empty directories.

## Chapter Select

Documented on [TCRF](https://tcrf.net/Star_Fox_Adventures#Chapter_Select). Of note, `save1.bin` is
not used at all — selecting Chapter 1 just begins a new game — but swapping `save1` with `save2`
reveals a save in the Walled City.

## Unused objects

In Moon Mountain Pass, there is a fuel cell (ID `0004BE3B`) which is assigned the same GameBit as
another (ID `0004BE3E`), but set to only appear in acts 9, 10, and 12, none of which are used.

### Bugged objects

- Standing at the entrance to the cave containing the shrine warp in SnowHorn Wastes, there are two
  fuel cells; collecting the rightmost one prevents a Bafomdad from appearing in the Walled City
  (buried near the landing pad) and vice versa, since both are assigned the same GameBit.
- Also in SnowHorn Wastes, there's a secret tunnel near the BribeClaw (use Tricky to open it)
  leading to two fuel cells, and another pair near the floating platforms above. Two of these cells
  share the same flag, so collecting one causes the other to disappear.

## globalmap.bin

Old version of the map grid. Same format but many differences. In general many maps were packed
tightly together on layer 0 before they gave up and moved some of them to other layers. Some
overlap in ways that don't make sense, like a chunk of the Drakor boss arena blocking the path
between ThornTail Hollow and Moon Mountain Pass.

## LACTIONS.bin

- "Light Actions"; the file contains 1024 fixed 0x28-byte records.
- Earlier testing only observed the no-op `getLActions` wrapper. LFXEmitter
  actively loads placement-selected rows, keeps them in per-object state, and
  caches the most recently loaded row.
- Replacing the file with an empty one may appear harmless when no relevant
  LFXEmitter is active, but it does not establish that the file is unused.

## GAMETEXT.bin

Old version of Gametext with lots of old dialogue (the current format is documented on
[Gametext](Gametext); this section is only about the stale root-level copy).

`GAMETEXT.tab` appears to be an array of `u16` offsets, presumably needing to be multiplied by
something. First few entries:

```
0006 02B0 0000 0000  0502 0370 0404 0602
0402 0201 0101 0000  0802 0802 020E 0203
050E 0702 0202 0202  0203 0202 0202 0202
0202 0202 0203 0209  0217 0107 0202 0204
```

...up to `2C0` is like this, maybe a different format (like how the final version has the
character definitions). From `2C0` on:

```
0026 009E 0096 0028  006A 0034 0028 0020
0030 000E 0000 0000  00A4 0016 00AA 0048
0022 0190 002A 004C  0074 0120 009E 002C
0048 0026 0018 0036  0042 0050 0020 002A
```

These look more like lengths than offsets; the pattern repeats through the entire file.

First few texts in the `.bin`:

- `0000`-`0009`: all zeros
- `000A`: CONTINUE
- `0013`: SAVE
- `0018`: Game Time
- `0022`: Complete
- ...probably same phrases system as final...
- `003A`: THE GREAT BATTLE
- `0061`: Chomp Chomp...
- `0194`: HEEL

From `00B2` to `0193` is `00CD 0000 0001 0002 ... 006F`, then more texts. The texts look like they
might have a header. `0194` and `00B4` aren't in `GAMETEXT.tab` at all. At `0C42`:
`05 DC FF 2D 04 97 FF 19 00 "(Queen moan)"` — GameText ID `05DC` doesn't seem to exist in the
final game; `0497` is completely unrelated. These could just be control codes: `05 rr gg bb`,
`04 rr gg bb`, `00 text`.

## SCREENS.bin

- Contains images that would have once been shown fullscreen.
- Two images, with text "Krystal's Adventure" and "Sabre's Adventure".
- Format: `u16 width, u16 height, 12 bytes unknown, raw pixel data` (probably YUYV 16-bit).
- `SCREENS.tab` gives their offset and size.
- DLL 0x11 was responsible for loading and displaying them, but no longer displays them.

## Misc unsorted notes

- The map `dbay` has `animtest` as its parent.
- Game text ID `0x02AC` is used for the map screen but has some unused entries: Level *(unused)*,
  Choose, Select, Map *(unused)*, Rotate, Zoom, OK, Exit, "TM & 2002 NINTENDO GAME BY RARE"
  *(unused)*.
- The FrontEnd `gametext.bin` contains unused strings: "Xmas Mode", "Play full game",
  "GAME SELECT", "PREVIOUSLY ON".
- Warping to map `0x5D` gives AnimTest, an empty void where Krystal does an animation and then
  the map reloads; after a few reloads it crashes. The objects present are `objPtr = 816B22A0`,
  `nObjs = 7`. The actual AnimTest map isn't empty; this probably happens because the game is way
  out of bounds — no idea why it triggers animations though.
- Code `042AB410 38000958` replaces the fireball spell with an unused(?) rapid-fire laser whose
  model is missing.

The seven objects seen from that out-of-bounds AnimTest load:

| Obj | Address  | Name        | ID   | Def  | ModelPtr | XPos     | YPos    | ZPos     | Ch |
|----:|----------|-------------|------|------|----------|---------:|--------:|---------:|---:|
| 0   | 812FB760 | KP_Transpor | 0025 | 059A | 812FB86C | +19373.80|   +23.00| +30080.00| 0  |
| 1   | 812FB880 | KP_Transpor | 0025 | 059A | 812FB98C | +13440.99|  +973.00| +30397.73| 0  |
| 2   | 81337280 | AnimCamera  | 0010 | 001E | 8133738C | +13440.99|  +973.00| +30397.73| 0  |
| 3   | 8133B8C0 | Override    | 0010 | 0006 | 8133B9CC | +13440.99|  +973.00| +30397.73| 0  |
| 4   | 81336D00 | Override    | 0010 | 0006 | 81336E0C | +13440.99|  +973.00| +30397.73| 0  |
| 5   | 806EDD00 | Krystal     | 0001 | 001F | 806EDE0C | +13440.99|  +973.00| +30397.73| 2  |
| 6   | 8133A420 | fox_shield  | 0030 | 0773 | 8133A52C | +13440.99|  +973.00| +30397.73| 0  |

Map ID is 5, "ZNot Used - Krazoa Palace". Coords `+13440.99 +973.00 +30397.73`, Cell `14 40 @ 8960
25600`, Anim `035A`. Neither `ID` nor `Def` is the index in `OBJECTS.bin`. The old Krazoa Palace map
does contain more objects, but they don't load because there's no map geometry below them (or
anywhere at all).

## Display debug objects

Hugo Peters discovered this AR/Gecko code for the kiosk demo to make invisible debug/trigger
objects visible:

```
0402f438 5480003c
0402f43c 901e0044
0402f440 60000000
0402f444 60000000
```

Gilgamesh ported it to other versions:

| Version | Code |
|---|---|
| US 1.00 | `0402D820 5480003C` / `0402D824 901E0044` / `0402D828 60000000` / `0402D82C 60000000` |
| US 1.01 | `0402D918 5480003C` / `0402D91C 901E0044` / `0402D920 60000000` / `0402D924 60000000` |
| Japan 1.00 | `0402D840 5480003C` / `0402D844 901E0044` / `0402D848 60000000` / `0402D84C 60000000` |
| Japan 1.01 | `0402D918 5480003C` / `0402D91C 901E0044` / `0402D920 60000000` / `0402D924 60000000` |
| Europe 1.00 & 1.01 | `0402D9B4 5480003C` / `0402D9B8 901E0044` / `0402D9BC 60000000` / `0402D9C0 60000000` |

---

## In this codebase

This page's topics were checked against `src/`, `include/`, and `config/GSAE01/symbols.txt`. Where
another wiki page already covers the ground in depth, this section links there instead of
repeating it.

### Already covered elsewhere — see these pages first

- **The whole disc-root fileId table** (`DLLS.bin/tab`, `VOXOBJ.bin/tab`, `CACHEFON.bin`,
  `GAMETEXT.bin/tab`, `SCREENS.bin/tab`, `LACTIONS.bin`, `globalma.bin`, and every other file named
  above) is enumerated with concrete `fileId`s, consumer call sites, and a "not found"/"confirmed"
  verdict per file in **[Files](Files)** ("Per-file findings" table + `enum MldfFileId`
  ready-to-adopt block). In particular Files.md already independently confirms: `DLLS.bin`/`DLLS.tab`
  are table-slot-only in `pi_dolphin.c` and **not the same file** as this page's "old N64 DLLS.tab"
  (though the wiki's leftover ids `0x58`/`0xAB` do map onto real do-nothing stubs
  `src/main/dll/dll_0058_dummy58.c` / `dll_00AB_projdummy.c`); `VOXOBJ.tab` (fileId `0x35`) *is*
  actually loaded at voxmaps init (`gVoxMapsMapList`, a `-1`-terminated list — see Files.md) while
  `VOXOBJ.bin` (`0x36`) has no consumer; `GAMETEXT.bin/tab` (fileId
  `0x13`/`0x14`) is the stale root copy, distinct from the live `gametext/%s/%s.bin` system.
- **The live Gametext format** (character/message structs, control codes, font slots) is
  **[Gametext](Gametext)** — this page's `GAMETEXT.bin` hex dump is the *old, unused* predecessor of
  that format, not the same thing.
- **`dbay`'s parent is `animtest`**, and the full `frontend`/`frontend2`/`swapcircle`/`dfalls`/
  `animtest`/Krazoa-Palace-as-map-5 picture (directory list, parent-map table, unused/mismatched
  directories) is already worked out in **[MapList](MapList)** ("Parent Maps", "Unused/Mismatched
  Directories" sections) — including the exact `05|dbay|animtest` parent-map row this page's misc
  note refers to.
- **The DLL ID → `dll_XXXX_*.c` file mapping** (including which IDs are confirmed-inert stub
  descriptors) is **[DLLs](DLLs)**.
- The **`CHAPBITS.bin`** angle of "chapter select" (a *different*, unrelated disc-root file this
  page doesn't mention but that shares the word "chapter") is covered in **[ChapBits](ChapBits)**,
  which also gives a shorter version of the Chapter Select cross-reference below.

### LACTIONS.bin — the no-op wrapper and the active consumer

`getLActions` (`src/main/render.c`) does match the earlier no-op observation:

```c
int getLActions(void* source, void* target, u16 index, int arg3, int arg4, int arg5) {
    void* buf = mmAlloc(0x28, -1, 0);
    getTabEntry(buf, MLDF_FILEID_LACTIONS_BIN, index * 0x28, 0x28);
    mm_free(buf);
    return 0;
}
```

FileId `0xc` is `LACTIONS.bin` (`sResourceFileNameLactionsBin`, see [Files](Files)). It allocates a
`0x28`-byte buffer, reads exactly one `0x28`-byte record at `idx * 0x28`, frees the buffer without
ever reading from it, and unconditionally returns 0. `getLActions` is called from ~15 object files
(`src/dlls/objects/504_WM_Galleon/WM_Galleon.c`,
`src/dlls/objects/496_SB_KyteCage/SB_KyteCage.c`,
`src/dlls/objects/298_CFCrate/CFCrate.c`, `src/dlls/objects/294/294.c`, etc.),
always with the same no-op result.

That wrapper is not the whole file-access story. Slot 301,
`src/dlls/objects/301_LFXEmitter/LFXEmitter.c`, allocates a 0x28-byte row,
loads `placement.actionIndex * 0x28` from fileId `0xc`, retains the row in its
0x124-byte per-object state, and caches it. The extracted EN file is exactly
40960 bytes (1024 rows); populated rows use +0x0E as a one-based row index,
matching the emitter's cache comparison. The active consumer disproves the
page's former conclusion that every read was immediately discarded.

### SCREENS.bin / DLL 0x11 — exact match, plus one refinement

`gScreensInterface = Resource_Acquire(0x11, 3)` (`src/main/gameloop.c:827`) and its file-comment in
`src/main/dll/dll_0011_screens.c:12-16` both confirm "DLL 0x11 ... loading ... the screens overlay
buffer": `screens_show(int id)` (`dll_0011_screens.c:221-249`,
`screens_show = .text:0x800EA650` in `config/GSAE01/symbols.txt`) does
`loadAssetFileById(&asset, 0x19)` (SCREENS.tab) to find `offset`/`size`, then
`getTabEntry(buf, 0x18, offset, size)` (SCREENS.bin) to load the raw bytes into a heap buffer,
caching the loaded id (`lbl_803DD4AC`) and a "dirty" flag (`lbl_803DD4A8`).

The refinement: **nothing else in the decompiled source ever reads `lbl_803DD4A0`/`lbl_803DD4A4`/
`lbl_803DD4A8`** (a repo-wide grep for those four globals only turns up this one file) — i.e. the
overlay is still *loaded* into RAM every time `screens_show` is called, exactly as the wiki says,
but there is no surviving render/blit call site anywhere that actually draws the pixels. That's a
direct, from-source confirmation of "no longer displays them," not just an absence-of-evidence
guess. `gameUpdate()` (`src/main/gameloop.c:932`) does call into `gScreensInterface`'s vtable slot
`+0xc` every frame while the screen isn't blanked, so the DLL's frame tick still runs — it just never
reaches pixels. The two specific images ("Krystal's Adventure"/"Sabre's Adventure", the YUYV pixel
format) aren't decoded anywhere in this codebase; not found.

### Chapter Select — the debug menu is fully decompiled

`src/main/dll/dll_0035_saveselectscreen.c` (DLL 0x35, `saveselectscreen`) implements exactly the
TCRF-documented feature:

```c
#define SAVE_SELECT_PANEL_CHAPTER_SELECT 4 /* chapter (act) select */
void saveSelectGoToChapterSelect(void);
```

`saveSelectGoToChapterSelect()` (`dll_0035_saveselectscreen.c:275-314`) switches the save-select
screen into a 6-entry chapter list, hiding entries past
`saveFileSelect_saveSlots[...].cheatFlag` (the unlock-progress gate) — called from
`SaveSelectScreen_run` on player input (line 152, 408) and reachable via a debug/cheat path (line
659). Selecting an entry calls `saveSelectSetSlot(sel, slot)` (line 727).

The save file path format confirms the wiki's `save1.bin`/`save2.bin` naming directly:

```c
char sSaveGameBinPathFormat[] = "/savegame/save%d.bin"; // dll_0035_saveselectscreen.c:125
```

used at `dll_0035_saveselectscreen.c:632`: `sprintf(buf, sSaveGameBinPathFormat, lbl_803DD6C4);
data = loadFileByPath(buf, 0, 0);` — i.e. this is a real, still-present code path that opens
`/savegame/saveN.bin` files, matching the wiki's `card`/`savegame` directory description in
[Files](Files) ("save game files used for debug chapter select"). Whether `saveN.bin` index 1
specifically is "never read" (the wiki's `save1.bin` claim) wasn't traced further — the format
string and dispatch are confirmed, the per-slot behavioral quirk is not.

### GameBit system — the machinery the fuel-cell bugs ride on

`include/main/gamebits.h` (`mainGetBit`/`mainSetBits`, `enum GameBitId`) is this codebase's
extensively-documented persistent-flag engine — the exact mechanism the wiki's "assigned the same
GameBit" fuel-cell bugs exploit. The fuel cell object itself is decompiled:

- `src/dlls/objects/291_fuelCell/fuelCell.c` (DLL 0x123, `fuelCell` in [DLLs](DLLs)) uses
  `FuelCellPlacement { ...; s16 offBit /* 0x1e */; s16 onBit /* 0x20 */; }` from
  `include/dlls/objects/291_fuelCell.h` — per-placement GameBit ids,
  i.e. exactly the "assigned the same GameBit" collision surface the wiki describes: two placements
  in `OBJECTS.bin`/level data sharing an `offBit`/`onBit` value would exhibit precisely the bugs
  reported ("collecting one causes the other to disappear").
- `GAMEBIT_ITEM_FuelCell_CantGet` (`0xE97`) is a *global* bit (not per-placement), unrelated to the
  per-instance bug.
- The "acts" the wiki mentions (fuel cell "set to only appear in acts 9, 10, and 12") map onto this
  codebase's per-map act-counter system: `gSaveGameMapActBits[120]` /
  `SaveGame_getMapAct(int idx)` / `SaveGame_gplaySetAct(int idx, int act)`
  (`src/main/dll/dll_0017_savegame.c`) — each map has one `GameBit` bank holding a 0-N "act" counter,
  and `include/main/gamebits.h` separately documents several per-area `*_ActNo` ids (e.g.
  `GAMEBIT_SH_ActNo`, 17 areas total, size-4/4-bit counters). No per-object "which acts is this
  placement visible in" gating code was found in `src/dlls/objects/291_fuelCell/fuelCell.c` itself — that check likely
  happens generically at object-spawn time (placement data, not fuelcell-specific code), and wasn't
  traced further here.
- The specific object ids from the wiki (`0004BE3B`, `0004BE3E`) are raw `OBJECTS.bin`/level
  placement data, not symbols — **not found** as named constants anywhere in source (expected: this
  is data, not code).

### Severed subsystems — APIs whose output nothing in the retail binary reads

A relocation scan over every retail object (all loads/stores/address-takes of every data symbol)
proves the following state is **stored but never loaded, and never address-taken, anywhere in the
shipped binary**. Each is a mechanism whose consumer was severed before ship; the writers remain.
These are pinned as-is — the decompiled source must keep writing them and must not "fix" the
missing reader.

- **Per-object render-override channel** (`src/main/objhits.c`, API in
  `include/main/objprint_api.h`): `objSetGlowColor` (backing `gObjGlowColorRed/Green/Blue/Alpha/`
  `Enabled`), `objSetColorFilter` (`gObjColorFilterRed/Green/Blue/Enabled`) and
  `objSetModelMatrixOverride` (`gObjModelMatrixOverride`) are pure setters into 11 globals with
  zero loads binary-wide. They still have *live callers*: the baddie red damage-glow
  (`202.c`/`Kaldachom.c`/`DBstealerwo.c`/`247.c` pass `RGBA(200,0,0,glowAlpha)`), per-object tints
  (`MSPlantingS.c`, `687.c`, `439.c`, `VFP_lavapoo.c`) and boss/platform model-matrix overrides
  (`DR_EarthWar.c`, `DR_CloudRun.c`, `211.c`, `626.c`, `625.c`, `DIMSnowHorn.c`) — all of which
  therefore do nothing in retail. Same class as the severed voxel-map link (see [Maps](Maps)).
- **Sub-map carrier latch** (`src/main/shader.c`): on entering a `mapType`-1 moving sub-map, the
  map resolver copies `MapInfoRecord.objType` (the sub-map's carrier object type — see
  [Files](Files)) and the sub-map's id into the halfword pair `lbl_803DCEB4`/`lbl_803DCEB6`; all 7
  retail references to the pair are `sth` stores. The mechanism that would consume the carrier
  object id (the wiki's "which object to use as the player" from older builds) is gone.
- **Root-motion rotation export** (`src/main/model.c`, `ObjModel_UpdateAnimMatrices`): the sampled
  root-joint rotation triple is stored to `gModelRootRotX/Y/Z` and never read — only the
  translation half of root motion is consumed.
- **Packed (delta-compressed) animation-resource path** (`src/main/model.c` /
  `src/main/render.c`): `loadAndDecompressDataFile`'s ANIM/PREANIM cases gate an unpack step on
  `ObjModel_IsPackedResource`, which retail hardcodes to `return 0` — so
  `ObjModel_UnpackResourcePayload` and its workers `modelRenderCopyPackedSamples` /
  `modelRenderDecodeAdpcm` are unreachable. The data side agrees: the record flag bit the
  unpacker clears (`0x20` at record offset 1) is set in none of the 2051 animation records
  shipped on the rev1 disc (see [Animation](Animation)). The compressor was retired before ship;
  the decoder shipped stubbed off.
- **RomCurve type-0x16 / type-0x17 query slots** (`src/dlls/engine/20_Hcurves/Hcurves_romcurve.c`):
  `curves_findNearestOfType16` (vtable slot 0x48) and `curves_findEnclosingLoopOfType17` (slot
  0x50) have zero callers binary-wide, and the retail romlists ship zero type-0x16 curve records
  and exactly one (link-less) type-0x17 record — severed on both the code and data sides (see
  [Curves](Curves)).
- **ObjSeq camera-override rotation half** (`src/dlls/engine/2/2.c`,
  `ObjSeq_SetCameraTransformOverride`): the setter stores an 8-global pose —
  `gObjSeqCameraOverrideActive`/`PosX/Y/Z` *are* read by the sequence-camera builder, but the
  rotation triple `gObjSeqCameraOverrideRotX/Y/Z` and `gObjSeqCameraOverrideW` have zero loads
  binary-wide (retail relocs: exactly the four stores). The one live caller —
  `src/dlls/objects/666_ARWArwing/ARWArwing.c`, which passes a fully aim-adjusted rotation for the
  Arwing cutscene camera — therefore overrides only the camera *position*; the camera keeps
  deriving rotation from the source object's own `anim.rotX/Y/Z`.
- **Map-scripted water-FX kill switch** (`src/track/intersect.c`, `waterFxSetDisabled`): stores
  `gWaterFxDisabled`, which nothing reads — the splash/ripple renderer has no check. The live
  caller (`src/dlls/objects/294/294.c`, driven by a placement parameter) therefore cannot disable
  water FX; the call's only retail effect is the ripple/splash buffer reset the setter itself
  performs when *re-enabling*.
- **expgfx texture-free guard + pool-update arbitration** (`src/dlls/engine/10_expgfx/expgfx.c`,
  `src/dlls/engine/11/11.c`): `gExpgfxTextureFreeInProgress` is set to 1/0 around every
  `textureFree` in expgfx (8 bracket pairs, 16 stores) and `gExpgfxUpdatingActivePools` is
  written by *two* DLLs (expgfx writes 1/0, engine 11 writes 2/0) — classic in-progress /
  who-owns-the-shared-pools flags whose checker no longer exists; zero loads binary-wide.
- **Per-frame effect-oscillator exports** (15 DLLs, 30 globals): every `gEffectN Sin/Sine/Osc`
  value pair (`gEffect1SineWaveA/B` … `gEffect20SineValue0/1` in engines 26-35 and 41-45, plus
  `gModgfxSineWaveA/B` in 33 and `gPartfxOscSine0/1` in 14) is recomputed from its live phase
  counter every frame and stored to a global nothing reads — a copy-pasted oscillator-export
  template whose consumers were retired; only the phase counters are live.
- **HUD screen-width offset half** (`src/dlls/engine/0/0.c`): game-UI init computes
  `gGameUiScreenWidthOffset = width - 320` and `gGameUiScreenHeightOffset = height - 240`; the
  height half is consumed, the width half has zero loads binary-wide.
- One decomp-side artifact found (and left in place) by the same scan: `shadowGetSunMagnitude`
  (`src/main/shadow_dolphin.c`) is an **invented reader** — a zero-caller static wrapping
  `gSunMagnitude` that provably does not exist in the retail object (retail `.text` is 0x1CCC
  bytes vs 0x1D04 with it, and retail carries exactly one `gSunMagnitude` reloc, the store).
  It cannot simply be deleted: it currently front-mints this TU's `.sdata2` entries (an `f32`
  0.0 and the s16-to-f32 conversion double) so the pool lines up with retail's layout, whose
  first `1.0f` sits *after* the conversion double even though retail code uses it earlier —
  i.e. retail's `lbl_803DEC58`/`lbl_803DEC68` look like declared constants, not first-use pool
  literals. Removing the helper costs 88 bytes of matched `.sdata2`. Flagged for a data-lane
  re-derivation of the TU's real constant declarations; for census purposes `gSunMagnitude` is
  a store-only latch like the entries above.

Most other store-only globals found by the same scan are one-off debug snapshot mirrors (e.g.
`gShadowTrackTriangleCount`, `gTrackTriangleCount`, `gDvdLastDriveStatus`,
`gObjSeqCurrentTrackId`, the `lbl_803DCEE8`-`lbl_803DCF18` shadow-track snapshot cluster) rather
than whole severed mechanisms; a value stored *and used in-register* in the same function also
shows up as "never loaded" without being dead (`gPauseMenuHoloRotY`, `gPlayerMoveTargetYaw`,
`gMoonFxDayNo`, `gNewShadowLightAngleY`, `gCamForceBehindTraceDistance`,
`gWmLevelControlBlendedLightIntensity` are that class), so store-only alone is not proof of
severance — the entries above are additionally pure-setter/pure-latch shaped.

### Not found in this codebase

- `KP_Transpor`, `Override`, `AnimCamera`, `fox_shield` as debug-print object-name strings — no
  matching name table found (these are presumably printed via a DLL-name lookup baked into a
  debugger/tool external to the retail binary, not something the game itself carries as strings).
- The `042AB410 38000958` fireball→laser AR code and the "Display debug objects" AR/Gecko codes:
  these are raw binary-patch addresses for specific disc revisions (US 1.00/1.01, Japan, Europe,
  the kiosk demo). This project's `symbols.txt`/build targets US `GSAE01`; the naive AR→RAM
  decode of `042AB410` (`0x802AB410`) doesn't land on any known symbol in
  `config/GSAE01/symbols.txt`, and no attempt was made to map the other regions' addresses — out of
  scope for a static source cross-reference.
- `/modules`, `/musyxbin`: these are disc-root files/directories with no code path opening them by
  name anywhere in `src/`/`include/` — consistent with the wiki's "unused"/"empty" characterization.
  (Not re-derived here since [Files](Files) already did the exhaustive string/file-table search for
  the disc-root file list; these two aren't files in that table at all, being bare directories.)

## Ready-to-adopt code

Nothing here needs a *new* enum/struct in the general case — `include/main/gamebits.h` and
[DLLs](DLLs)/[Files](Files)'s ready-to-adopt blocks already cover the GameBit and fileId ids this
page touches. The one page-specific piece worth naming, if a maintainer later works on
`dll_0035_saveselectscreen.c`, is the save-select sub-panel enum (currently `#define`s, not an
`enum`, and only the one file uses it):

```c
/* gSaveSelectPanelIndex values (currently #defines in dll_0035_saveselectscreen.c:36-40). */
enum SaveSelectPanelId {
    SAVE_SELECT_PANEL_CHOOSE_SLOT   = 0, /* pick a save slot */
    SAVE_SELECT_PANEL_OPEN_FILE     = 1, /* opened file: continue / save */
    SAVE_SELECT_PANEL_SLOT_ACTION   = 2, /* copy / erase slot action */
    SAVE_SELECT_PANEL_CONFIRM_ERASE = 3, /* confirm-erase prompt */
    SAVE_SELECT_PANEL_CHAPTER_SELECT = 4, /* chapter (act) select - the TCRF "Chapter Select" debug menu */
};
```

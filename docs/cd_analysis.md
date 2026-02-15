# CD Subsystem Analysis (`cd.c` / `cd.h`)

## Table of Contents

1. [Architecture Overview](#1-architecture-overview)
2. [Memory Map & Address Relationships](#2-memory-map--address-relationships)
3. [Status Flag Bit Definitions](#3-status-flag-bit-definitions)
4. [Function-by-Function Analysis](#4-function-by-function-analysis)
5. [Suggested Name Improvements](#5-suggested-name-improvements)
6. [Struct Field Analysis & Renaming](#6-struct-field-analysis--renaming)
7. [Bugs & Inconsistencies](#7-bugs--inconsistencies)
8. [Suggested Macros / Inline Helpers](#8-suggested-macros--inline-helpers)
9. [Header Architecture: `#define` vs `extern`](#9-header-architecture-define-vs-extern)
10. [Misc Observations](#10-misc-observations)

---

## 1. Architecture Overview

This file implements a **custom CD-ROM command queue and state machine** layered on top of Sony's Psy-Q `libcd` API. The core design:

- A **16-entry circular command queue** (`commandQueue`) with read/write indices masked to `0x0F`.
- A **central state struct** (`CdSystem`) at fixed address `0x801ED800` that holds all runtime state.
- An **initialization/recovery state machine** (4 states, 0–3) that handles error recovery and mode re-initialization across multiple VSync frames.
- **VSync-based timing** throughout — all delays are counted in VSync frames (1/60s NTSC or 1/50s PAL).
- A **resource table** (`SKCDPOSE_DAT`) at `0x801ED998` that maps resource indices to disc locations (CdlLOC + size).
- **Scratchpad RAM** (`0x1F800000`) used as temporary DMA/communication buffer for streaming.

The sentinel value `0xFFFE` for `resourceIndex` means "use `defaultCdResource`" (the manually-configured entry at struct offset 0x190), while `0xFFFF` means "use entry at `g_defaultCdResource`" (address `0x801ED990`, which is `g_cdSystem.defaultCdResource`).

---

## 2. Memory Map & Address Relationships

Many `extern` globals in `cd.h` are actually **aliases to fields within `g_cdSystem`**. This is a decompilation artifact — the decompiler (likely Ghidra) didn't recognize the struct and emitted them as separate symbols.

### Address-to-Field Mapping (base = `0x801ED800`)

| Symbol | Address | Offset | Actual Struct Field |
|---|---|---|---|
| `g_cdSystem` | `0x801ED800` | `0x000` | (base of struct) |
| `g_cdStatusByte3` | `0x801ED802` | `0x002` | `statusFlags.bytes.b2` |
| `g_cdAudioEnabled` | `0x801ED804` | `0x004` | `audioEnabled` |
| `g_playbackState` | `0x801ED805` | `0x005` | `playbackState` |
| `g_playbackFlag` | `0x801ED806` | `0x006` | `playbackFlag` |
| `g_initState` | `0x801ED815` | `0x015` | `initState` |
| `g_size` | `0x801ED828` | `0x028` | `size` |
| `g_otherQueue` | `0x801ED8F0` | `0x0F0` | `commandQueue.items[11]` |
| `g_cdVSyncTimestamp` | `0x801ED94C` | `0x14C` | `vsyncTimestamp` |
| `g_cdSetModeArg` | `0x801ED950` | `0x150` | `setModeBuffer` |
| `g_cdStatusByte` | `0x801ED960` | `0x160` | `statusByte` |
| `g_cdSyncCallbackResult` | `0x801ED968` | `0x168` | `previousSyncCallback` |
| `g_cdReadyCallbackResult` | `0x801ED96C` | `0x16C` | `previousReadyCallback` |
| `g_defaultCdResource` | `0x801ED990` | `0x190` | `defaultCdResource` |
| `g_SKCDPOSE_DAT` | `0x801ED998` | `0x198` | (immediately after struct, separate table) |
| `g_cdResource176` | `0x801EDF18` | — | `g_SKCDPOSE_DAT.resources[176]` |

### Truly External Globals (NOT in `g_cdSystem`)

| Symbol | Address | Notes |
|---|---|---|
| `g_cdAudioReady` | `0x801ED591` | Separate — 0x26F bytes *before* `g_cdSystem`. Likely part of a movie/FMV playback system. |

### Other Notable Addresses

| Address | Used In | Meaning |
|---|---|---|
| `0x1F800000` | `CD_InitializeSubsystem`, `CD_StreamData` | PS1 **Scratchpad RAM** (1 KB fast SRAM) |
| `0x801ED500` | `CD_ResetSystem` | Likely an **FMV/movie playback system struct** — offsets 0x38/0x3C store saved DecDCTout/DrawSync callbacks |
| `0x801ED958` | `CD_ExecuteCommand` | = `g_cdSystem` + 0x158 = `commandParamBuffer` field |

---

## 3. Status Flag Bit Definitions

The `statusFlags.word` field is a bitfield controlling system state. Based on usage analysis:

| Bit | Mask | Set When | Cleared When | Likely Meaning |
|---|---|---|---|---|
| 0 | `0x01` | `CD_HandleSyncError` | Init state machine completes | **Error / needs recovery** |
| 1 | `0x02` | (unknown) | Init state machine | **Recovery in progress** (paired with bit 0) |
| 2 | `0x04` | (unknown) | `CD_InitializeSubsystem` | Unknown |
| 3 | `0x08` | (unknown) | `CD_InitializeSubsystem` | **Hardware not ready** — gates entire `UpdateAndProcessQueue` |
| 4 | `0x10` | (reading?) | Stop/reset functions | **Active read/shell open** |
| 5 | `0x20` | (unknown) | `CD_InitializeSubsystem` | Unknown |
| 6 | `0x40` | (unknown) | `CD_PauseAndClearState` | **Busy/locked** — blocks `CD_EnqueueCommand` |
| 7 | `0x80` | (persistent) | Never cleared in init | **Persistent system flag** (preserved across all resets) |

The sequential single-bit clearing in `CD_InitializeSubsystem` (`& ~0x01`, `& ~0x02`, etc.) is a **compiler-matching trick** — it forces the compiler to emit individual AND instructions matching the original binary, rather than a single `& 0x80` which would produce different machine code.

---

## 4. Function-by-Function Analysis

### `CD_InitializeSubsystem` (0x80011710) — 100% match

**Better name:** `CD_Init`

**Purpose:** Full cold-start initialization of the CD-ROM subsystem.

**Flow:**
1. Spin-waits on `CdInit()` until hardware ready
2. Disables debug output, saves previous callbacks
3. Resets all state in `g_cdSystem` to zero/defaults
4. Clears status flags (preserving only bit 7)
5. Initializes all 16 command queue entries with scratchpad address defaults
6. Sets CD mode to `0xA0` = **double speed (0x80) + auto-pause (0x20)**
   - *Note: comment says "auto-pause" but 0xA0 is actually `CdlModeSpeed | 0x20`. On PS1, 0x20 in mode byte means "XA-ADPCM playback on". This should be verified.*
7. Gets current status via `CdControlB(CdlNop, ...)` (command 1)
8. If shell-open flag set, waits for disc ready
9. Applies mode via `CdControlB(CdlSetmode, ...)` (command 14)
10. Records VSync timestamp

**Issues:**
- The `do {} while (0)` between saving the two callbacks is explicitly noted as a compiler trick for branch delay slot matching. This is correct PSX decomp practice.
- Comment says "0xa0 = double speed with auto-pause" but the PS1 mode byte 0xA0 more precisely means `CdlModeSpeed (0x80) | 0x20`. The 0x20 bit enables XA-ADPCM audio filter, not auto-pause (which is 0x02). **The comment is likely wrong.**
- `g_otherQueue` is a misleading name — it's just `commandQueue.items[11]`, used as an anchor for a descending pointer loop that initializes all 16 queue slots via `queueItem[4]` indexing.

---

### `CD_PauseAndClearState` (0x800118DC) — 100% match

**Better name:** `CD_Stop`

**Purpose:** Full stop — pauses the drive, clears all state, flushes buffers.

**Flow:**
1. If audio is playing, calls `CD_ResetSystem()` first (stops FMV/audio)
2. Clears bit 6 (busy flag)
3. Removes callbacks
4. Issues `CdlPause` command (blocking)
5. Resets nearly all `g_cdSystem` fields to zero
6. Clears bit 4 (active read flag)
7. Records VSync timestamp
8. Clears status bytes b1/b2 and queue indices
9. Calls `CdFlush()` to discard any pending data

**Notes:**
- The original comment references "func_80014014" — this is `CD_ResetSystem` at address `0x80014014`. The reference is stale.
- Nearly identical state-clearing code to `CD_InitializeSubsystem` — candidate for shared helper (see macros section).

---

### `CD_StreamData` (0x800119C0) — 90.71% match

**Better name:** `CD_ReadAndDecompress` or `CD_StreamDecompressToRam`

**Purpose:** Reads data from a disc resource and decompresses it to a destination buffer. This is the main streaming entry point.

**Flow:**
1. Waits for queue to empty
2. Sets up scratchpad communication area (ready flag, counters)
3. Enqueues a read command (command 6) with `FUN_80014888` as the sector-ready callback
4. Enters a polling loop:
   - Waits up to 30 VSync frames for the ready flag
   - On timeout, calls `CD_UpdateAndProcessQueue` and retries
   - When data arrives, calls `CD_DecompressData` to decompress chunks
   - Handles buffer wrapping for circular read buffer

**Notes:**
- Uses scratchpad RAM (`0x1F800000`) as a shared communication area between the main thread and the CD callback:
  - Offset `0x00`: ready flag (set to 1 by callback when sector available)
  - Offset `0x01`: secondary flag
  - Offset `0x04`: buffer start address
  - Offset `0x08`: buffer end address
  - Offset `0x0C`: current buffer size / bytes available
  - Offset `0x14`: processed count
  - Offset `0x18`: total transferred
- The 30-frame timeout (0.5 seconds) is a reasonable guard against stuck reads.
- `FUN_80014888` is the **sector data callback** — better name: `CD_OnStreamSectorReady`.
- The magic number `280` in `srcEnd` calculation is likely one CD sector (2048 bytes raw, but maybe 280 words = 1120 bytes for a compressed sector chunk?). This needs verification.
- This function is only 90.71% — the omitted lines likely contain buffer wraparound logic and the completion check.

---

### `CD_EnqueueCommand` (0x800120A0) — 97.54% match

**Better name:** `CD_QueueCommand`

**Purpose:** Submits a command to the circular command queue, or executes immediately if queue is empty.

**Flow:**
1. Rejects if bit 6 (busy) is set
2. Resolves resource index: `0xFFFF` → `g_defaultCdResource` (address 0x801ED990); else looks up in `g_SKCDPOSE_DAT.resources[]`
3. If no command is currently executing (`currentCommand == 0`), executes immediately
4. Otherwise enqueues into circular buffer
5. Returns `resourceEntry->dataSize`

**Notes:**
- The address `0x801ED990` is `g_cdSystem.defaultCdResource`, confirming that `0xFFFF` means "use the default/manual location."
- Command 6 appears to be a "read" operation based on usage in `CD_StreamData` and `CD_InitLocationEntries`.
- Return value is always the `dataSize` from the resource entry, regardless of success or queue state.

---

### `CD_UpdateAndProcessQueue` (0x800122C0) — 87.92% match

**Better name:** `CD_Update` or `CD_Tick`

**Purpose:** Main per-frame update function. Processes the command queue, manages retries, runs the init/recovery state machine, and handles audio.

**Flow:**
1. Checks bit 3 — if set, CD hardware is not ready, returns early
2. Manages playback state (loop counter → playback state transitions)
3. Checks VSync timeout (30 frames = 0x1E)
4. Runs error recovery states (switch on `g_initState`: states 5–8, 32)
5. If no errors, processes the command queue (dequeues and executes next command)
6. Manages audio if enabled

**Notes:**
- This is the most complex function and the least-matched (87.92%). The heavy use of goto, block labels, and fragmented control flow suggests Ghidra struggled with the nested conditionals.
- The 30-frame (0x1E) timeout appears repeatedly — this is the CD retry timeout.
- `g_initState` values 5–8, 32 are recovery substates within `CD_UpdateAndProcessQueue`, distinct from the 0–3 states in `CD_ProcessInitStateMachine`. This is confusing and suggests the state machine is split across two functions.
- The variable `indexDiff` is the **queue depth** (write - read, masked to 0x0F).
- Return value is the number of pending commands (0 = idle).

---

### `CD_ProcessInitStateMachine` (0x80012B48) — 94.89% match

**Better name:** `CD_RecoveryStateMachine` or `CD_ReInitStep`

**Purpose:** Asynchronous state machine for CD re-initialization after errors. Called from `CD_UpdateAndProcessQueue`.

**States:**
- **State 0:** `CdFlush()`, advance after 1 VSync frame
- **State 1:** Set mode 0xA0, install callbacks, send `CdlSetmode`, wait 4 frames
- **State 2:** Set filter parameters (1,1), advance immediately
- **State 3:** Wait for `syncComplete` flag or 30-frame timeout, then dispatch follow-up command:
  - Command `0x11` → `CdlDemute` (unmute CD audio)
  - Command `0x12` → `CdlPause` 
  - Others → `CdlSetfilter` + reset to command `0x10`

**Notes:**
- This is clearly an **error recovery sequence** — flush, reconfigure mode, reapply filter, then resume the interrupted operation.
- The comment's TODO about "why state 3 subtracts 30 from timestamp" — this is likely to prevent a double-wait: after the 30-frame timeout, it adjusts the timestamp so the next state doesn't wait an additional 30 frames.
- Commands 0x10/0x11/0x12 are **custom internal command IDs**, not Psy-Q CdlXxx constants. They denote which operation to resume after recovery.

---

### `CD_SyncCallback_Handler` / `CD_SyncCallback_Handler2` (ASM-only)

**Better names:** `CD_OnCommandComplete` / `CD_OnCommandAcknowledge`

These are interrupt callbacks registered with `CdSyncCallback`. They fire when the CD drive acknowledges/completes a command. Still in assembly form — likely handles status byte parsing, error detection, and setting `syncComplete`.

---

### `CD_ReadyCallback` (ASM-only)

**Better name:** `CD_OnDataReady`

Fires when CD data is ready to be read. Registered via `CdReadyCallback`. Likely triggers DMA transfer of sector data.

---

### `CD_HandleSectorReadComplete` (ASM-only)

**Better name:** `CD_OnSectorTransferred`

Called after a sector has been fully transferred. Likely updates buffer pointers and decrements remaining sector count.

---

### `CD_ExecuteCommand` (0x80013A00) — 96.59% match

**Better name:** `CD_DispatchCommand` or `CD_IssueCommand`

**Purpose:** Translates a queued command into actual Psy-Q `CdControlF` calls.

**Flow:**
1. Special handling for `CdlSeekL` — skips past already-queued seeks
2. For read commands (`CdlReadN`/`CdlReadS`): sets up read callbacks and parameters
3. For other commands: extracts the location from the queue entry and copies to parameter buffer
4. Dispatches via `CdControlF` (non-blocking CD command)

**Notes:**
- `executionMode` appears to control how the callback is set:
  - Mode 0: Full setup (sync + ready callbacks)
  - Mode 1: No ready callback (fire-and-forget)
  - Mode 2+: Direct execution with current callbacks
- The address `0x801ED958` passed to `CdControlF` is `g_cdSystem.commandParamBuffer` (offset 0x158).
- The seek-skip logic (checking for chained `CdlSeekL` commands) is an optimization — if multiple seeks are queued, it skips to the last one.

---

### `FUN_80013d74` (ASM-only)

**Better name:** `CD_HandleCommandRetry` (speculative)

Located between `CD_ExecuteCommand` and `CD_WaitForQueueEmpty`. Based on address ordering and context, this likely handles **command retry logic** or **error-triggered re-execution** of the current command.

---

### `CD_WaitForQueueEmpty` (0x80013F2C) — 100% match

**Better name:** `CD_Sync` or `CD_DrainQueue`

**Purpose:** Blocking wait until all queued commands complete.

Simple polling loop — calls `CD_UpdateAndProcessQueue()` each frame until it returns 0. The `VSync(0)` call in the loop body prevents busy-waiting by yielding until the next frame.

---

### `CD_HandleSyncError` (0x80013F64) — 100% match

**Better name:** `CD_OnError`

**Purpose:** Called when a CD sync/command error occurs. Resets to error recovery state.

**Flow:**
1. Removes all callbacks
2. Resets `initState` to 0 (triggers recovery state machine)
3. Sets bit 0 (error flag) in status
4. Clears active command and retry counters
5. Clears bit 4 (active read)
6. Records VSync timestamp for recovery timing

---

### `CD_SetAudioVolume` (0x80013FD0) — 100% match

**Better name:** Fine as-is, or `CD_SetVolume`

**Purpose:** Sets CD-XA/CDDA audio volume for left or right channel.

Uses `CdlATV` structure to configure the mixing matrix. The `do {} while(0)` wrapper is again a compiler-matching trick. Channel parameter: 0 = left, non-zero = right. The `.val3 = 0` zeroes cross-channel mixing.

---

### `CD_ResetSystem` (0x80014014) — 100% match

**Better name:** `CD_StopAudioAndVideo` or `CD_ShutdownPlayback`

**Purpose:** Stops any ongoing CD audio/FMV playback and restores previous video callbacks.

**Flow:**
1. Restores `DecDCToutCallback` and `DrawSyncCallback` from saved values at `0x801ED500` (FMV system struct)
2. Removes CD callbacks
3. Issues `CdlPause` (command 9 — **but wait, CdlPause is actually command 0x09!** So this is correct.)
4. If `g_cdAudioReady`, calls some cleanup (omitted)
5. Resets audio/playback state

**Critical observation:** The address `0x801ED500` is an **FMV/movie playback system struct**. This function is the bridge between the CD subsystem and the video playback subsystem. The fields at offsets 0x38 (`ptr[0x0E]`) and 0x3C (`ptr[0x0F]`) store the saved DecDCT and DrawSync callback pointers that were replaced when FMV playback started.

---

### `CD_CanQueueResourceIndex` (0x800140D4) — 98.89% match

**Better name:** `CD_IsQueueAvailable` or `CD_CheckQueueCapacity`

**Purpose:** Checks if the command queue has space and whether a given resource index is already queued.

**Flow:**
1. Calculates queue depth: `(writeIndex - readIndex) & 0x0F`
2. If queue has entries, scans for duplicate resource index
3. Returns 1 if command can be queued, likely 0 if duplicate or queue full

**Note:** The `-1` after the mask suggests it's checking for at least one *free* slot (queue full = 16 entries, so depth 15 means one slot left).

---

### `CD_InitLocationEntries` (0x80014140) — 100% match

**Better name:** `CD_SetupManualLocation` or `CD_SeekToLba`

**Purpose:** Sets up a manual disc location and triggers a seek/read.

**Flow:**
1. VSync timing check — waits if too soon after last command (3-frame guard)
2. Clears `defaultCdResource.location`, sets `dataSize`
3. Converts LBA to MSF via `CdIntToPos`
4. Enqueues command 6 with `g_SKCDPOSE_DAT` as destination
5. Waits for completion
6. Sets audio volume to 128 (50%) on right channel

**Notes:**
- This appears used for **game data loading** (not streaming).
- The audio volume set at the end is strange after a data read — possibly this is called before starting audio playback, and the volume setup is a side-effect done here for convenience.
- The `vsyncOffset = -3` creates a 3-frame minimum gap between commands.

---

## 5. Suggested Name Improvements

### Functions

| Current Name | Suggested Name | Rationale |
|---|---|---|
| `CD_InitializeSubsystem` | `CD_Init` | Standard name; "subsystem" is redundant |
| `CD_PauseAndClearState` | `CD_Stop` | It does a full stop, not just pause |
| `CD_StreamData` | `CD_ReadAndDecompress` | Describes actual behavior |
| `CD_EnqueueCommand` | `CD_QueueCommand` | Simpler |
| `CD_UpdateAndProcessQueue` | `CD_Update` | Called every frame; canonical name |
| `CD_ProcessInitStateMachine` | `CD_RecoveryStep` | It's error recovery, not init |
| `CD_ExecuteCommand` | `CD_DispatchCommand` | Dispatches to Psy-Q; "execute" is vague |
| `CD_WaitForQueueEmpty` | `CD_Sync` | PS1 convention (cf. `DrawSync`, `VSync`) |
| `CD_HandleSyncError` | `CD_OnError` | Simpler |
| `CD_ResetSystem` | `CD_StopPlayback` | It specifically stops audio/FMV |
| `CD_CanQueueResourceIndex` | `CD_IsQueueAvailable` | Clearer intent |
| `CD_InitLocationEntries` | `CD_SeekToLba` | Describes the action |
| `FUN_80014888` | `CD_OnStreamSectorReady` | Streaming sector callback |
| `FUN_80014ad0` | `CD_StreamCleanup` (speculative) | Unknown; needs ASM analysis |
| `FUN_80013d74` | `CD_RetryCommand` (speculative) | Between ExecuteCommand and WaitForQueue |
| `FUN_80022400` | (unknown — different module) | Address is in a different code region |
| `FUN_80140d48` | **Likely address typo** | `0x80140d48` is way outside the executable range; should this be `0x80014d48`? |

### Globals

| Current Name | Suggested Name | Notes |
|---|---|---|
| `g_cdStatusByte3` | `g_cdRecoveryStatus` or just use `g_cdSystem.statusFlags.bytes.b2` | "byte 3" but it's actually byte 2 (offset 0x02) |
| `g_playbackFlag` | `g_cdPlaybackActive` | |
| `g_playbackState` | `g_cdPlaybackPhase` | Distinguishes from the flag |
| `g_initState` | `g_cdRecoveryState` | It's recovery, not init |
| `g_size` | `g_cdTransferSize` | Too generic |
| `g_otherQueue` | (remove — see section 9) | Misleading; it's just `commandQueue.items[11]` |
| `g_SKCDPOSE_DAT` | `g_cdResourceTable` | Descriptive name. SKCDPOSE likely = "CD Position Table" from the disc image tooling |
| `g_cdResource176` | `g_cdResourceTableEnd` or `g_cdLastResource` | It's `g_cdResourceTable.resources[176]` |

---

## 6. Struct Field Analysis & Renaming

### `CdSystem` — Offset Verification & Suggested Names

| Offset | Current Name | Type | Suggested Name | Notes |
|---|---|---|---|---|
| 0x00 | `statusFlags` | `CdStatusFlags` | `flags` | OK, but see bit definitions above |
| 0x04 | `audioEnabled` | `u8` | `isAudioEnabled` | Boolean |
| 0x05 | `playbackState` | `u8` | `playbackPhase` | 0=idle, 1=playing |
| 0x06 | `playbackFlag` | `u8` | `isPlaying` | Boolean-like |
| 0x07 | `u_7` | `u8` | `_pad07` | Likely padding |
| 0x08 | `currentResourceIndex` | `u16` | `activeResourceIdx` | |
| 0x0A | `u_a` | `u8` | `_pad0A` | |
| 0x0B | `u_b` | `u8` | `_pad0B` | |
| 0x0C | `currentDataSize` | `u32` | `bytesRead` | Tracks progress |
| 0x10 | `targetDataSize` | `u32` | `totalBytes` | Expected total |
| 0x14 | `syncComplete` | `u8` | `isSyncComplete` | Boolean |
| 0x15 | `initState` | `u8` | `recoveryState` | State machine for error recovery |
| 0x16 | `currentCommand` | `u8` | `activeCommand` | Currently executing CD command |
| 0x17 | `initCommand` | `u8` | `pendingRecoveryCmd` | Command to resume after recovery |
| 0x18 | `retryCount` | `u8` | `maxRetries` | Max retry attempts |
| 0x19 | `retryCounter` | `u8` | `currentRetry` | Current retry attempt |
| 0x1A | `lastCommand` | `u8` | `prevCommand` | |
| 0x1B | `u_1b` | `u8` | `_pad1B` | |
| 0x1C | `resourceIndex` | `u16` | `queuedResourceIdx` | 0xFFFE = invalid sentinel |
| 0x1E | `u_1e` | `u8` | `_pad1E` | |
| 0x1F | `u_1f` | `u8` | `_pad1F` | |
| 0x20 | `dstBuffer` | `u32` | `readDestAddr` | DMA destination |
| 0x24 | `callback` | `u32` | `completionCallback` | Should be a function pointer type |
| 0x28 | `size` | `u32` | `sectorCount` or `transferSize` | Too generic |
| 0x2C | `sizeCopy` | `u32` | `originalSize` | Backup of size before read begins? |
| 0x30 | `dstBuffer2` | `u32` | `currentWritePtr` | Likely updated during transfers |
| 0x34 | `loopCounter` | `u32` | `audioLoopCount` | Number of audio loops remaining |
| 0x38 | `queueReadIndex` | `u32` | `queueHead` | Consumer index |
| 0x3C | `queueWriteIndex` | `u32` | `queueTail` | Producer index |
| 0x40 | `commandQueue` | `CdCommandQueue` | — | 16 × 16-byte entries = 256 bytes |
| 0x140 | `readSectorBuffer` | `u32` | `sectorBufferAddr` | |
| 0x144 | `u_144` | `u32` | `sectorBufferSize` (speculative) | |
| 0x148 | `u_148` | `u32` | `_unk148` | |
| 0x14C | `vsyncTimestamp` | `u32` | `lastVsyncFrame` | VSync frame counter snapshot |
| 0x150 | `setModeBuffer` | `u8` | `cdMode` | Byte sent to CdlSetmode |
| 0x151–0x153 | `u_151`–`u_153` | `u8` | `cdModeParams[3]` | Padding for mode command buffer |
| 0x154 | `modeParams` | `u8` | `filterFile` | CdlSetfilter file param |
| 0x155 | `u_155` | `u8` | `filterChannel` | CdlSetfilter channel param |
| 0x156–0x157 | `u_156`–`u_157` | `u8` | `_filterPad[2]` | |
| 0x158 | `commandParamBuffer` | `u32` | `seekLocation` | CdlLOC packed as u32, passed to CdControlF |
| 0x15C | `readParams` | `u32` | `readModeParams` | |
| 0x160 | `statusByte` | `u8` | `driveStatus` | Raw CD status byte from CdlNop |
| 0x161 | `filterModeFlags` | `u8` | `filterFlags` | |
| 0x162 | `u_162` | `u32` | `_unk162` | |
| 0x166 | `u_166` | `u16` | `_unk166` | |
| 0x168 | `previousSyncCallback` | `CdlCB` | `savedSyncCb` | |
| 0x16C | `previousReadyCallback` | `CdlCB` | `savedReadyCb` | |
| 0x170–0x18F | `u_170`–`u_18c` | various | (unknown) | 32 bytes of unknowns; might be result buffers for CD responses |
| 0x190 | `defaultCdResource` | `CdResourceEntry` | `manualResource` | Used when resourceIndex = 0xFFFF/0xFFFE |

### `CdStatusArray` — **UNUSED, DELETE IT**

Not referenced anywhere in the code.

### `CdCommandQueueItem`

| Field | Suggested Change |
|---|---|
| `callback` | Should be typed as `void (*)(int, u_int)` or similar, not bare `unsigned int` |
| `dstBuffer` | Should be `void*` or at least `u32` with a comment that it's an address |

### `SKCDPOSE_DAT`

**Rename to:** `CdResourceTable`

The `unknown[45065]` field suggests there's more data after the 178 resource entries — possibly a file allocation table, padding to sector boundary, or other disc metadata. The value 45065 should be checked: 178 entries × 8 bytes = 1424. If the total structure is at a known size, the unknown portion should be documented.

---

## 7. Bugs & Inconsistencies

### 1. Duplicate `extern` with different types
```c
extern u_char g_cdAudioReady;   // line ~103
extern u8 g_cdAudioReady;       // line ~106
```
While `u_char` and `u8` are both `unsigned char`, this is still a duplicate declaration that should be cleaned up. **Remove one.**

### 2. `g_cdStatusByte3` naming
The name says "byte 3" but the address `0x801ED802` is at offset 0x02 = `statusFlags.bytes.b2`. This is confusing. Either rename to `g_cdStatusByte2` or (better) eliminate it in favor of direct struct access.

### 3. Mode byte comment
`CD_InitializeSubsystem` comment says `0xa0` = "double speed with auto-pause". On PS1:
- `0x80` = `CdlModeSpeed` (double speed)
- `0x20` = XA-ADPCM audio filter enabled
- `0x02` = auto-pause

So `0xA0` = double speed + XA audio, **NOT** auto-pause.

### 4. `CD_ResetSystem` `CdControlB(9U, ...)` comment
The comment flow says this is `CdlPause`. `CdlPause` = 9. ✓ This is correct.

### 5. `FUN_80140d48` suspicious address
The address `0x80140d48` is far outside the normal executable range (`0x80011xxx`–`0x80022xxx`). This might be:
- A **typo** for `0x80014d48`
- A **BIOS function** (unlikely, BIOS is typically `0xA0`/`0xB0`/`0xC0` tables)
- An **overlay** function loaded by the game
- Verify against the actual disc binary.

### 6. Inconsistent use of struct fields vs globals
Code mixes `g_cdSystem.audioEnabled` with `g_cdAudioEnabled`, `g_cdSystem.initState` with `g_initState`, etc. The original binary likely used one or the other consistently — the decompiler split them. Choose one convention.

### 7. `CD_PauseAndClearState` doesn't clear `audioEnabled`
`CD_InitializeSubsystem` clears `audioEnabled`, but `CD_PauseAndClearState` does not. This may be intentional (audio state is handled by `CD_ResetSystem` called at the top), but it's worth verifying.

### 8. `CD_ReadyCallback` prototype mismatch
```c
void CD_ReadyCallback(char mode);  // in cd.h
```
But `CdReadyCallback` from Psy-Q expects/returns `CdlCB` (a function pointer). The prototype in cd.h declares `CD_ReadyCallback` as a function taking a `char` — this is the actual callback handler that *gets called by* `CdReadyCallback`, not a wrapper around it. The name is confusing since it matches the Psy-Q API name. Consider renaming to `CD_OnDataReady`.

---

## 8. Suggested Macros / Inline Helpers

### 1. Invalid Resource Index Sentinel

Used in `CD_InitializeSubsystem`, `CD_PauseAndClearState`, `CD_EnqueueCommand`:

```c
#define CD_RESOURCE_INDEX_INVALID   0xFFFE
#define CD_RESOURCE_INDEX_DEFAULT   0xFFFF
```

### 2. Scratchpad Address

Used in `CD_InitializeSubsystem`, `CD_StreamData`:

```c
#define SCRATCHPAD_BASE     0x1F800000
```

### 3. CD Mode

```c
#define CD_MODE_DOUBLE_SPEED_XA   0xA0  /* CdlModeSpeed | XA Audio */
```

### 4. VSync Timeout

The value `30` (0x1E) is used as a timeout in `CD_StreamData`, `CD_UpdateAndProcessQueue`, `CD_ProcessInitStateMachine`:

```c
#define CD_TIMEOUT_FRAMES   30  /* ~0.5 seconds at 60fps */
```

### 5. Status Flag Helpers

This would clean up the bitfield manipulation scattered everywhere:

```c
#define CD_FLAG_ERROR       0x01
#define CD_FLAG_RECOVERING  0x02
#define CD_FLAG_HW_NOTREADY 0x08
#define CD_FLAG_READING     0x10
#define CD_FLAG_BUSY        0x40
#define CD_FLAG_PERSISTENT  0x80

#define CD_SET_FLAG(f)    (g_cdSystem.statusFlags.word |= (f))
#define CD_CLR_FLAG(f)    (g_cdSystem.statusFlags.word &= ~(f))
#define CD_HAS_FLAG(f)    (g_cdSystem.statusFlags.word & (f))
```

**Caution:** Only use these macros if they produce identical machine code. The sequential `& ~0x01`, `& ~0x02` etc. in `CD_InitializeSubsystem` is deliberately done that way for binary matching. A combined `& 0x80` would NOT match.

### 6. State Reset (NOT recommended yet)

`CD_InitializeSubsystem` and `CD_PauseAndClearState` share ~15 lines of identical state clearing. However, since the fields cleared differ slightly and the order matters for binary matching, extracting a common helper would likely break matching. **Document the pattern but don't refactor until all functions match 100%.**

---

## 9. Header Architecture: `#define` vs `extern`

### Current Approach

```c
#define g_cdSystem      (*(struct CdSystem*)0x801ed800)
#define g_SKCDPOSE_DAT  (*(struct SKCDPOSE_DAT*)0x801ed998)
#define g_otherQueue    (*(CdCommandQueueItem*)0x801ed8f0)
```

### Why This Works

The `#define` approach dereferences a hardcoded pointer, which:
- Produces identical machine code to a global variable access on MIPS (both use `lui`/`lw` sequences)
- Works on decomp.me without a linker script
- Gives the compiler full type information

### Recommended Improvement: Conditional Compilation

```c
#ifdef DECOMP_ME
  /* Hard-coded addresses for decomp.me scratch context */
  #define g_cdSystem      (*(struct CdSystem*)0x801ed800)
  #define g_SKCDPOSE_DAT  (*(struct SKCDPOSE_DAT*)0x801ed998)
#else
  /* Resolved by linker from symbol_addrs / linker script */
  extern CdSystem g_cdSystem;
  extern SKCDPOSE_DAT g_SKCDPOSE_DAT;
#endif
```

For the main build, the linker script (generated from `symbol_addrs.txt` by splat) already places these at the correct addresses. Using `extern` is cleaner and allows the toolchain to verify symbol resolution.

### Eliminating `g_otherQueue`

`g_otherQueue` is `commandQueue.items[11]` — it should not be a public symbol at all. It's an implementation detail of the queue initialization loop. Options:

1. **Local variable in `CD_InitializeSubsystem`:**
   ```c
   volatile CdCommandQueueItem *queueItem = &g_cdSystem.commandQueue.items[11];
   ```

2. **Macro within cd.c only (not in header):**
   ```c
   #define QUEUE_INIT_ANCHOR (&g_cdSystem.commandQueue.items[11])
   ```

Either way, remove it from cd.h.

### Replacing Global Aliases with Struct Field Macros

For globals that are actually struct fields:

```c
/* Replace extern declarations with accessor macros */
#define g_cdAudioEnabled        g_cdSystem.audioEnabled
#define g_playbackState         g_cdSystem.playbackState
#define g_playbackFlag          g_cdSystem.playbackFlag
#define g_cdStatusByte3         g_cdSystem.statusFlags.bytes.b2
#define g_initState             g_cdSystem.initState
#define g_size                  g_cdSystem.size
#define g_cdVSyncTimestamp      g_cdSystem.vsyncTimestamp
#define g_cdStatusByte          g_cdSystem.statusByte
#define g_cdSyncCallbackResult  g_cdSystem.previousSyncCallback
#define g_cdReadyCallbackResult g_cdSystem.previousReadyCallback
```

This gives backward-compatible access while making the relationship explicit. Both the `extern` style and the `#define` style will produce identical code on MIPS.

**Keep as true externs** (not in `g_cdSystem`):
- `g_cdAudioReady` at `0x801ED591` — legitimately separate

---

## 10. Misc Observations

### Command ID Table (Internal)

| ID | Used In | Meaning |
|---|---|---|
| 6 | `CD_EnqueueCommand`, `CD_StreamData`, `CD_InitLocationEntries` | Read data / seek to location |
| 0x10 | `CD_ProcessInitStateMachine` | Recovery: set filter (then reset to 0x10) |
| 0x11 | `CD_ProcessInitStateMachine` | Recovery: demute audio |
| 0x12 | `CD_ProcessInitStateMachine` | Recovery: pause |

### `SKCDPOSE_DAT` Name Origin

Based on typical PS1 disc image tooling conventions:
- **SK** = unknown prefix (game-specific?)
- **CD** = CD-ROM
- **POSE** = Position
- **DAT** = Data

So `SKCDPOSE.DAT` is likely a file on the disc containing a **CD position table** — an array mapping logical resource indices to physical disc locations (minute:second:frame + size). Renamed suggestion: `g_cdResourceTable` or `g_discLocationTable`.

### Scratchpad Communication Protocol in `CD_StreamData`

The scratchpad layout at `0x1F800000` forms a simple producer-consumer protocol between the main thread and the CD interrupt callbacks:

| Offset | Size | Direction | Meaning |
|---|---|---|---|
| 0x00 | 1 | CB → Main | Ready flag (1 = data available) |
| 0x01 | 1 | CB → Main | Secondary flag / error? |
| 0x04 | 4 | CB → Main | Buffer start address |
| 0x08 | 4 | Main ← → CB | Buffer cursor / read pointer |
| 0x0C | 4 | CB → Main | Bytes available in buffer |
| 0x14 | 4 | Main → CB | Bytes consumed |
| 0x18 | 4 | Main | Total bytes transferred |

This is an efficient zero-copy scheme — the callback fills a circular buffer and signals readiness via scratchpad, avoiding main-RAM cache coherency issues since scratchpad is uncached.

### Function Ordering by Address

```
0x80011710  CD_InitializeSubsystem     (CD_Init)
0x800118DC  CD_PauseAndClearState      (CD_Stop)
0x800119C0  CD_StreamData              (CD_ReadAndDecompress)
0x800120A0  CD_EnqueueCommand          (CD_QueueCommand)
0x800122C0  CD_UpdateAndProcessQueue   (CD_Update)
0x80012B48  CD_ProcessInitStateMachine (CD_RecoveryStep)
0x80012E88  CD_SyncCallback_Handler2   (CD_OnCommandAcknowledge)
0x800130A8  CD_SyncCallback_Handler    (CD_OnCommandComplete)
0x80013584  CD_ReadyCallback           (CD_OnDataReady)
0x80013744  CD_HandleSectorReadComplete(CD_OnSectorTransferred)
0x80013A00  CD_ExecuteCommand          (CD_DispatchCommand)
0x80013D74  FUN_80013d74               (CD_RetryCommand?)
0x80013F2C  CD_WaitForQueueEmpty       (CD_Sync)
0x80013F64  CD_HandleSyncError         (CD_OnError)
0x80013FD0  CD_SetAudioVolume          (CD_SetVolume)
0x80014014  CD_ResetSystem             (CD_StopPlayback)
0x800140D4  CD_CanQueueResourceIndex   (CD_IsQueueAvailable)
0x80014140  CD_InitLocationEntries     (CD_SeekToLba)
0x80014448  CD_DecompressData          (in decompression.c)
0x80014888  FUN_80014888               (CD_OnStreamSectorReady)
0x80014AD0  FUN_80014ad0               (CD_StreamCleanup?)
```

This tight address range (`0x80011710`–`0x80014AD0`, ~13 KB) confirms all these functions are part of a single compilation unit (`cd.c`), with `CD_DecompressData` being at the boundary (possibly pulled from `decompression.c` or inlined).

# NTSC-J (SLPS-02170) configuration

Splat configuration for the Japanese release. This folder mirrors
[`config/us/`](../us/) and is selected with `make VERSION=jp`.

Nothing here is wired up yet. Expected contents once the port starts:

| Path | Contents |
|---|---|
| `SLPS_021.70.yaml` | Splat config for the main executable (`disc/jp/SLPS_021.70`) |
| `overlays/<NAME>.BIN.yaml` | One splat config per overlay in `disc/jp/BIN/` |
| `symbols/*_symbol_addrs.txt` | Same symbol names as `config/us/symbols/`, with JP addresses |
| `relocations/*_reloc_addrs.txt` | JP relocation overrides |

Every path inside the yamls points at the `jp` trees (`disc/jp/`, `asm/jp/`,
`linker/jp/`, `assets/jp/`, `build/jp/`) and uses `base_path: ../../` (main
executable) or `base_path: ../../../` (overlays).

Reference files:

| File | SHA-1 |
|---|---|
| `SLPS_021.70` | `b067188a92e4de9a4db7bb7e5343c757e9884bfa` |
| Disc image (`.bin`) | `7a314615be8a482cf3f81b4101cc19aaa738f36d` |

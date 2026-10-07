# Build asset tools

[Tools index](../../README.md)

These modules parse and rebuild the structured assets embedded in the game
binaries. `mk/assets.mk` invokes their command-line interfaces, and the adapters
in `tools/splat_ext/` use their parsers during extraction. Keep the module paths
stable unless both integrations are updated together.

| Area | Modules |
| --- | --- |
| Texture data | `psx_tim`, `tim_upload_table` |
| Sprite and text graphics | `sprite_layout`, `sprite_animation`, `uv_rect_table`, `glyph_metrics`, `tab_cursor_layout` |
| Indexed data | `asset_offset_table`, `index_boundaries`, `index_map`, `u8_sequence` |
| Game and menu data | `game_state_template`, `save_layout_table`, `starting_weapon_table`, `name_entry_resource` |

Each module has a CLI. From the repository root:

```sh
python3 tools/data/formats/psx_tim.py --help
python3 tools/data/formats/psx_tim.py info /path/to/image.tim --json
python3 tools/data/formats/psx_tim.py roundtrip /path/to/image.tim
python3 tools/data/formats/sprite_layout.py --help
make test-assets
```

The format-specific builders and validators are also available through
`make validate-assets` in the build container. Tests live in `tests/` and import
the parsers through `tools.data.formats`; they contain synthetic data, not game assets.

For scene `.IMG` files, use the separate [scene tools](../scenes/README.md).

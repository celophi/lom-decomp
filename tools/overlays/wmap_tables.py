"""Layouts of WMAP's fixed tables, as used by the C sources.

Record formats follow the C types that read each symbol. Counts come from the
C array bounds where there are some; the rest cover the stored run up to the
next resource. Sound effects, scripted input and handler tables have their own
readers in wmap.py.
"""

from dataclasses import dataclass


@dataclass(frozen=True)
class TableSpec:
    """One array: its symbol, little-endian record format, row count and what the code does with it."""

    symbol: str
    format: str
    count: int
    fields: tuple[str, ...] = ()
    note: str = ""
    resolve_symbols: bool = False


# libgpu primitives, as the templates are stored.
SPRT = ("I4B2h2BH2h", ("tag", "r0", "g0", "b0", "code", "x0", "y0", "u0", "v0", "clut", "w", "h"))
POLY_FT4 = ("I4B2h2BH2h2BH2h2BH2h2BH", (
    "tag", "r0", "g0", "b0", "code",
    "x0", "y0", "u0", "v0", "clut", "x1", "y1", "u1", "v1", "tpage",
    "x2", "y2", "u2", "v2", "pad1", "x3", "y3", "u3", "v3", "pad2",
))
POLY_G4 = ("I4B2h4B2h4B2h4B2h", (
    "tag", "r0", "g0", "b0", "code", "x0", "y0", "r1", "g1", "b1", "pad1", "x1", "y1",
    "r2", "g2", "b2", "pad2", "x2", "y2", "r3", "g3", "b3", "pad3", "x3", "y3",
))
# WmapMenuTriangle and WmapTriangle: a Gouraud triangle with its packet tag.
POLY_G3 = ("I4B2h4B2h4B2h", (
    "tag", "r0", "g0", "b0", "code", "x0", "y0", "r1", "g1", "b1", "pad1", "x1", "y1",
    "r2", "g2", "b2", "pad2", "x2", "y2",
))
# WmapQuadTemplate: four WmapQuadVertex, then four CVECTOR colors.
QUAD_TEMPLATE = ("16H16B", tuple(
    [f"{axis}{corner}" for corner in range(4) for axis in ("x", "y", "z", "pad")]
    + [f"{channel}{corner}" for corner in range(4) for channel in ("r", "g", "b", "cd")]
))
TRANSFER_FRAME = ("6h", ("x", "y", "shape", "texture", "sound_id", "foreground"))


def spec(symbol: str, layout: tuple[str, tuple[str, ...]], count: int, note: str) -> TableSpec:
    return TableSpec(symbol, layout[0], count, layout[1], note)


TABLES = (
    # The map game (wmap_map_display.c).
    spec("g_wmap_game_continue_prompt", SPRT, 1, "Sprite template for the map game's continue prompt."),
    spec("g_wmap_game_exit_prompt", SPRT, 1, "Sprite template for the game's exit prompt."),
    spec("g_wmap_game_score_label", SPRT, 1, "Sprite template for the score label."),
    spec("g_wmap_game_score_digit", SPRT, 1, "Sprite template for one score digit."),
    spec("g_wmap_game_round_label", SPRT, 1, "Sprite template for the round label."),
    spec("g_wmap_game_round_digit", SPRT, 1, "Sprite template for the round digit; the code sets u0 to round * 16."),
    spec("g_wmap_game_countdown_sprite", SPRT, 1, "Sprite template for the start countdown."),
    TableSpec("g_wmap_game_spawn_patterns", "9B", 8,
              note="One row per round: nonzero cells of the 3 x 3 game area may raise a land."),
    # Information panel, cell effects and land textures (wmap_map_display.c).
    TableSpec("g_wmap_information_glyphs", "4BH2B", 28, ("u", "v", "width", "height", "palette", "pad_06", "pad_07"),
              "Texture rectangle and palette of each information-panel glyph."),
    TableSpec("g_wmap_information_placements", "H2B", 313, ("x", "y", "glyph"),
              "Screen position and glyph of each information-panel label; glyph 0xFF is drawn from a runtime value."),
    TableSpec("g_wmap_information_group_starts", "i", 11,
              note="First placement of each panel group; each group ends where the next one starts."),
    spec("g_wmap_cell_effect_quads", QUAD_TEMPLATE, 16, "Vertices and corner colors of the cell effect quads."),
    TableSpec("D_800CBBE8", "4h8H2H", 17,
              ("x", "y", "width", "height", *[f"clut{index}" for index in range(8)], "tpage_x", "tpage_y"),
              "Land texture cache slots: the VRAM rectangle LoadImage fills, palettes and texture page."),
    TableSpec("g_wmap_land_quad_scales", "8h", 48, ("x0", "y0", "x1", "y1", "x2", "y2", "x3", "y3"),
              "Per-corner scale factors, 3 variants of 16 steps, for land transitions."),
    TableSpec("g_wmap_spirit_sequence_bounds", "B", 12,
              note="Start of each spirit's frame run in g_wmap_spirit_sequences; the stored run ends with padding."),
    TableSpec("g_wmap_spirit_sequences", "BB", 44, ("cell", "duration"),
              "Spirit animation frames: a 24 x 24 cell of a five-column sheet, then its duration."),
    # Artifact carousel and transfer paths (wmap_land_transition.c, wmap_land_preview.c).
    TableSpec("g_wmap_carousel_turn_frames", "i", 1, note="Carousel state; zero on disc."),
    TableSpec("g_wmap_carousel_turn_step", "i", 1, note="Carousel state; zero on disc."),
    TableSpec("g_wmap_carousel_frame", "i", 1, note="Carousel state; zero on disc."),
    TableSpec("g_wmap_artifact_positions", "2h", 12, ("x", "y"), "Screen position of each artifact carousel slot."),
    TableSpec("g_wmap_artifact_images", "4B6H4h", 64, (
        "u", "pad_01", "v", "pad_03", "width", "height", "carousel_x", "carousel_y",
        "shadow_x", "shadow_y", "offset_x", "offset_y", "anchor_x", "anchor_y",
    ), "Artifact texture rectangle and drawing offsets for the carousel, its shadow and the map."),
    TableSpec("g_wmap_carousel_translation", "4i", 1, ("vx", "vy", "vz", "pad"), "Carousel translation vector."),
    TableSpec("g_wmap_carousel_rotation", "4h", 1, ("vx", "vy", "vz", "pad"),
              "Carousel rotation; the code writes vy (also named D_800CC776) from the carousel angle table."),
    TableSpec("g_wmap_preview_travel_frames", "2h4B", 126, ("x", "y", "pad_04", "pad_05", "pad_06", "pad_07"),
              "Path from the map to the artifact carousel."),
    TableSpec("D_800CCBF4", "i", 65, note="First pickup frame of each artifact; the next entry minus one is the last."),
    spec("g_wmap_artifact_pickup_frames", TRANSFER_FRAME, 740, "Artifact pickup animation frames."),
    TableSpec("g_wmap_artifact_return_offsets", "i", 65, note="First return frame of each artifact."),
    spec("g_wmap_artifact_return_frames", TRANSFER_FRAME, 280, "Artifact return animation frames."),
    # Land layout (wmap_land_layout.c).
    TableSpec("g_wmap_land_attributes", "8BI", 64, (*[f"spirit_{index}" for index in range(8)], "flags"),
              "Spirit strengths and placement flags of each artifact."),
    TableSpec("g_wmap_spirit_sprites", "i", 57,
              note="Spirit influence sprites: component * 7 + level, plus a diagonal entry."),
    # Event lands (wmap_pathfinding.c) and map labels (wmap_map_labels.c).
    TableSpec("D_800D01B0", "B", 64, note="Land entered for each pending world event flag."),
    TableSpec("D_800D01F0", "B", 64, note="Music track index set for each pending world event flag."),
    TableSpec("D_800D0230", "i", 9, note="Label width scale for each cursor cell of the 3 x 3 selection."),
    TableSpec("D_800D0254", "i", 69, note="Label width of each land name, scaled by D_800D0230."),
    TableSpec("D_800D0368", "i", 1, note="Label palette row; the code switches it between 506 and 508."),
    TableSpec("D_800D036C", "h", 64,
              note="Label texture cell of each land: value / 14 picks a 112-pixel column, value % 14 a 16-pixel row."),
    TableSpec("D_800D03EC", "4B2H", 4, ("u", "pad_01", "v", "pad_03", "w", "h"),
              "Auxiliary label font glyphs (WmapLabelGlyph)."),
    TableSpec("D_800D040C", "2BH2B", 8, ("glyph", "pad_01", "x", "y", "pad_05"),
              "Placed glyphs of the auxiliary labels (WmapLabelChar)."),
    # Frame, input and backdrop templates (wmap_frame_render.c, wmap_main.c).
    TableSpec("D_800D043C", "I4B8h", 1, ("tag", "r0", "g0", "b0", "code", "x0", "y0", "x1", "y1", "x2", "y2", "x3", "y3"),
              "Flat quad template copied for full-screen fills."),
    TableSpec("D_800D0454", "I", 1, note="Pointer to the controller state block the menu reads; a fixed RAM address.",
              resolve_symbols=True),
    TableSpec("D_800D0550", "i", 1, note="Result of the last wmap_run_loop call."),
    spec("g_wmap_backdrop_front_quads", POLY_FT4, 4, "Front backdrop layer quads."),
    spec("g_wmap_backdrop_back_quads", POLY_FT4, 4, "Back backdrop layer quads."),
    spec("D_800D0694", POLY_FT4, 1, "Fade quad; the code steps its colour, and tests r0 through D_800D0698."),
    spec("D_800D06BC", POLY_G4, 1, "Gradient quad copied into the backdrop state."),
    TableSpec("g_wmap_backdrop_scroll", "i", 1, note="Backdrop scroll offset; zero on disc."),
    spec("D_800D06E4", SPRT, 1, "Prompt sprite template."),
    spec("g_wmap_menu_triangles", POLY_G3, 28, "Gouraud triangles of the menu cursor."),
    TableSpec("D_800D0A08", "B", 60, note="Menu cursor primitives, patched byte by byte by the menu code."),
    TableSpec("g_wmap_sprite_textures", "8B8H2H", 48, (
        *[f"unknown_{index:02x}" for index in range(8)], *[f"clut{index}" for index in range(8)], "tpage", "unknown_1a",
    ), "Texture page and palettes of each map sprite texture."),
    # Backdrop effect geometry (wmap_effect_backdrop.c).
    TableSpec("g_wmap_transition_mesh_vertices", "2h", 528, ("x", "y"), "Vertices of the backdrop triangles, three per triangle."),
    spec("g_wmap_transition_mesh_triangles_0", POLY_G3, 176, "Backdrop triangles; the code rewrites their vertices from g_wmap_transition_mesh_vertices."),
    TableSpec("g_wmap_transition_mesh_motion_0", "2i", 528, ("x", "y"), "First backdrop per-vertex motion table, 16.16 fixed point."),
    TableSpec("g_wmap_transition_mesh_motion_1", "2i", 528, ("x", "y"), "Second backdrop per-vertex motion table, 16.16 fixed point; zero on disc."),
    # Travel and land effects.
    TableSpec("g_wmap_route_headings", "h", 10, note="Heading the vehicle must face on each special travel route."),
    TableSpec("D_800D665C", "i", 6,
              note="Model index in the effect resource that land effect 27 draws, by animation frame."),
)

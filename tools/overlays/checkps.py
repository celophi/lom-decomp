#!/usr/bin/env python3
"""Export CHECKPS's embedded resources as files people can read.

The startup overlay has an AKAO sound bank, a TIM image and a few small
hardware and font tables. The warning text and quadrant signs sit in rodata
before the code. The build links all of those bytes unchanged; these exports
are for looking at the resources.

Like ADDHERO and CARDA, this tool finds its inputs through splat and the
version's symbol file. Each read_* function returns the parts it recognizes,
and the byte maps keep track of padding and anything still unidentified.

Example:

    python3 -m tools.overlays.checkps --version us assets/exports/us/overlays/checkps
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import re
import shutil
import struct
import sys
import tempfile

from tools.assets.psx_tim import TimBlock, TimImage
from tools.overlays import icon_set, png, splat_config, symbols
from tools.overlays.resources import (
    Blob, Part, byte_map_entry, cover_gaps, dump_yaml, hex_address, write_part,
)

REPO_ROOT = Path(__file__).resolve().parents[2]
OVERLAY_CONFIG = "overlays/CHECKPS.BIN.yaml"
SYMBOL_FILE = "symbols/checkps_symbol_addrs.txt"
CD_SOURCE = REPO_ROOT / "src/overlays/checkps/cdrom.c"

# Source layouts: AkaoBankHeader and AkaoArticulation in include/akao.h.
AKAO_HEADER_BYTES = 16
AKAO_BANK_HEADER_BYTES = 64
AKAO_ARTICULATION_BYTES = 16
CD_COMMAND_BYTES = 4  # CheckPSCdCommandDescriptor, cdrom.c
PATTERN_SIZE_COUNT = 17  # CHECKPS_PATTERN_SIZE_COUNT, pattern.c
PATTERN_QUADRANT_COUNT = 4  # CHECKPS_PATTERN_QUADRANT_COUNT, pattern.c
WARNING_BYTES = 60  # CHECKPS_HARDWARE_WARNING_SIZE, checkps_internal.h
GLYPH_CLUT_PREFIX_BYTES = 12  # g_glyph_clut_prefix, font.c


# ---------------------------------------------------------------------------
# Inputs and decoded parts


@dataclass(frozen=True)
class CheckpsSymbols:
    """Every address the tool needs, named in SYMBOL_NAMES below."""

    audio: int
    image: int
    commands: int
    status_register: int
    response_register: int
    data_register: int
    irq_register: int
    parameters: int
    response: int
    state: int
    irq_sum: int
    pattern_sizes: int
    decimal_glyphs: int
    hex_glyphs: int
    clut_prefix: int
    bss: int
    warning: int
    quadrant_signs: int

    @classmethod
    def load(cls, path: Path) -> CheckpsSymbols:
        named = symbols.load(path)
        missing = [name for name in SYMBOL_NAMES.values() if name not in named]
        if missing:
            raise ValueError(
                f"{path} has no {', '.join(missing)}. If a symbol was renamed, "
                "update SYMBOL_NAMES in tools/overlays/checkps.py."
            )
        return cls(**{key: named[name] for key, name in SYMBOL_NAMES.items()})


SYMBOL_NAMES = {
    "audio": "g_embedded_checkps_akao",
    "image": "g_checkps_image_asset",
    "commands": "g_cd_command_table",
    "status_register": "g_cd_status_register",
    "response_register": "g_cd_response_register",
    "data_register": "g_cd_data_register",
    "irq_register": "g_cd_irq_register",
    "parameters": "g_cd_command_parameters",
    "response": "g_cd_response",
    "state": "g_checkps_state",
    "irq_sum": "g_cd_irq_code_sum",
    "pattern_sizes": "g_hardware_pattern_size_table",
    "decimal_glyphs": "g_decimal_glyph_table",
    "hex_glyphs": "g_hex_glyph_table",
    "clut_prefix": "g_glyph_clut_prefix",
    "bss": "g_checkps_exit_reason",
    "warning": "g_hardware_modification_warning",
    "quadrant_signs": "g_hardware_pattern_vertex_signs",
}


@dataclass(frozen=True)
class Inputs:
    """Where one version's CHECKPS data and symbols live."""

    version: str
    config: Path
    assets: Path

    @property
    def symbol_file(self) -> Path:
        return self.config / SYMBOL_FILE

    @property
    def overlay_config(self) -> Path:
        return self.config / OVERLAY_CONFIG


@dataclass(frozen=True)
class Table:
    address: int
    values: dict[str, object]
    note: str = ""


@dataclass(frozen=True)
class Image:
    address: int
    tim: TimImage


@dataclass(frozen=True)
class AudioBank:
    address: int
    data: bytes
    samples: bytes
    metadata: dict[str, object]


def read_bytes(blob: Blob, address: int, size: int, what: str) -> bytes:
    """Read a bounded range, naming the resource if it falls outside the file."""
    start = blob.offset(address)
    if size < 0 or start < 0 or start + size > len(blob.data):
        raise ValueError(f"{what} is outside {blob.file_name}")
    return blob.data[start : start + size]


def enum_names(type_name: str) -> dict[int, str]:
    """Read CHECKPS's simple integer enums, including their implicit increments."""
    source = CD_SOURCE.read_text(encoding="ascii")
    match = re.search(r"typedef enum\s*\{([^}]+)\}\s*" + type_name + r";", source)
    if match is None:
        raise ValueError(f"{CD_SOURCE} has no {type_name}")
    names = {}
    value = 0
    for name, explicit in re.findall(r"\b(CHECKPS_\w+)\s*(?:=\s*(-?\d+))?", match[1]):
        if explicit:
            value = int(explicit)
        names[value] = name
        value += 1
    return names


# ---------------------------------------------------------------------------
# Reading the initialized data, in address order


def read_blob(blob: Blob[CheckpsSymbols]) -> list[Part]:
    """Read the resources and account for every byte of the initialized data."""
    parts = read_audio(blob)
    parts += read_image(blob)
    parts.append(read_cd_commands(blob))
    parts.append(read_cd_registers(blob))
    parts.append(read_cd_state(blob))
    parts.append(read_pattern_sizes(blob))
    parts.append(read_digits(blob))
    parts.append(read_clut_prefix(blob))
    return cover_gaps(blob, sorted(parts, key=lambda part: part.start))


def akao_header(data: bytes) -> dict[str, object]:
    """The common 16-byte header; keep its six timestamp bytes as stored."""
    if len(data) < AKAO_HEADER_BYTES or data[:4] != b"AKAO":
        raise ValueError("missing AKAO header")
    bank_id, length, reverb = struct.unpack_from("<HHH", data, 4)
    return {
        "id": bank_id, "length": length, "reverb_type": reverb,
        "timestamp_bytes": data[10:16].hex(" "),
    }


def repeated_word(blob: Blob, end: int, limit: int, name: str) -> list[Part]:
    """Some embedded resources repeat their last word before the next resource."""
    if end >= 4 and end + 4 <= limit and blob.data[end : end + 4] == blob.data[end - 4 : end]:
        return [Part(name, end, end + 4, note="Repeats the resource's last word.")]
    return []


def read_audio(blob: Blob[CheckpsSymbols]) -> list[Part]:
    """The two-section AKAO container: resident program data and an uploadable bank."""
    address = blob.symbols.audio
    raw = read_bytes(blob, address, blob.symbols.image - address, "AKAO container")
    if len(raw) < 12:
        raise ValueError("AKAO container header is truncated")
    count, program, bank = struct.unpack_from("<III", raw)
    if count != 2 or not 12 <= program < bank <= len(raw) - AKAO_BANK_HEADER_BYTES:
        raise ValueError("invalid CHECKPS AKAO section offsets")
    program_header = akao_header(raw[program:bank])
    header = akao_header(raw[bank:])
    destination, sample_size, bank_id, count = struct.unpack_from("<4I", raw, bank + 16)
    samples_start = bank + AKAO_BANK_HEADER_BYTES + count * AKAO_ARTICULATION_BYTES
    end = samples_start + sample_size
    if samples_start > len(raw) or end > len(raw):
        raise ValueError("AKAO articulation table or samples run past the next resource")
    articulations = []
    for index in range(count):
        position = bank + AKAO_BANK_HEADER_BYTES + index * AKAO_ARTICULATION_BYTES
        sample, loop, adsr, pitch = struct.unpack_from("<4I", raw, position)
        if sample >= sample_size or loop >= sample_size:
            raise ValueError(f"AKAO articulation {index} points outside the sample data")
        articulations.append({
            "index": bank_id + index,
            "sample_offset": f"0x{sample:X}", "loop_offset": f"0x{loop:X}",
            "adsr_word": hex_address(adsr), "pitch_misc_word": hex_address(pitch),
        })
    metadata = {
        "header": header,
        "spu_destination": hex_address(destination),
        "sample_size": sample_size,
        "first_articulation": bank_id,
        "articulations": articulations,
        "unknown_0x20": hex_address(struct.unpack_from("<I", raw, bank + 32)[0]),
        "reserved_bytes": raw[bank + 36 : bank + AKAO_BANK_HEADER_BYTES].hex(" "),
        "bank_file": "bank.akao", "samples_file": "samples.adpcm",
        "note": "Samples are SPU ADPCM. The program bytecode and bank are kept as stored; "
                "this export does not synthesize the sound effects.",
    }
    container = Table(address, {"sections": [
        {"offset": f"0x{program:X}", "file": "program.akao", "header": program_header},
        {"offset": f"0x{bank:X}", "file": "bank.akao", "metadata": "bank.yaml"},
    ]})
    start = blob.offset(address)
    content = AudioBank(address + bank, raw[bank:end], raw[samples_start:end], metadata)
    parts = [
        Part("AKAO container header", start, start + 12, "audio/container.yaml", container),
        Part("AKAO program", start + program, start + bank, "audio/program.akao", raw[program:bank]),
        Part("AKAO bank", start + bank, start + end, "audio/bank.yaml", content),
    ]
    parts += repeated_word(blob, start + end, blob.offset(blob.symbols.image), "AKAO trailing word")
    return parts


def read_image(blob: Blob[CheckpsSymbols]) -> list[Part]:
    """Read the complete 4bpp TIM, keeping every palette in the JP texture."""
    address = blob.symbols.image
    raw = read_bytes(blob, address, blob.symbols.commands - address, "TIM image")
    if len(raw) < 8 or struct.unpack_from("<II", raw) != (0x10, 8):
        raise ValueError("CHECKPS image is not a 4bpp TIM with a CLUT")
    clut, end = TimBlock.parse(raw, 8, "CLUT")
    pixels, end = TimBlock.parse(raw, end, "pixels")
    if not clut.payload or len(clut.payload) % 32 or not pixels.payload:
        raise ValueError("CHECKPS image needs complete 16-color palettes and pixels")
    tim = TimImage.parse(raw[:end])
    start = blob.offset(address)
    parts = [Part("TIM image", start, start + end, "image/image.yaml", Image(address, tim))]
    parts += repeated_word(blob, start + end, blob.offset(blob.symbols.commands), "TIM trailing word")
    return parts


def read_cd_commands(blob: Blob[CheckpsSymbols]) -> Part:
    """Each CD command's opcode, parameter count, response count and IRQ sum."""
    first, last = blob.symbols.commands, blob.symbols.status_register
    raw = read_bytes(blob, first, last - first, "CD commands")
    names = enum_names("CheckPSCdCommandIndex")
    if len(raw) != len(names) * CD_COMMAND_BYTES:
        raise ValueError("CD command table size does not match CheckPSCdCommandIndex")
    commands = []
    for index, (opcode, parameters, response, irq_sum) in enumerate(struct.iter_unpack("<4B", raw)):
        commands.append({
            "index": index, "name": names[index], "opcode": f"0x{opcode:02X}",
            "parameter_count": parameters, "response_count": response, "irq_code_sum_target": irq_sum,
        })
    content = Table(first, {"commands": commands})
    return Part("CD commands", blob.offset(first), blob.offset(last), "tables/cd_commands.yaml", content)


def read_cd_registers(blob: Blob[CheckpsSymbols]) -> Part:
    """The four initialized pointers to the CD controller's register window."""
    registers = []
    for key in ("status_register", "response_register", "data_register", "irq_register"):
        address = getattr(blob.symbols, key)
        value = struct.unpack("<I", read_bytes(blob, address, 4, key))[0]
        registers.append({
            "symbol": SYMBOL_NAMES[key],
            "address": hex_address(address),
            "value": hex_address(value),
        })
    first, last = blob.symbols.status_register, blob.symbols.parameters
    content = Table(first, {"registers": registers})
    return Part("CD register pointers", blob.offset(first), blob.offset(last), "tables/cd_registers.yaml", content)


def read_cd_state(blob: Blob[CheckpsSymbols]) -> Part:
    """Initial command/response storage and the CD check's state words."""
    names = blob.symbols
    parameters = read_bytes(blob, names.parameters, names.response - names.parameters, "CD parameter storage")
    response = read_bytes(blob, names.response, names.state - names.response, "CD response storage")
    state = struct.unpack("<i", read_bytes(blob, names.state, 4, "CD state"))[0]
    irq_sum = struct.unpack("<i", read_bytes(blob, names.irq_sum, 4, "CD IRQ sum"))[0]
    content = Table(names.parameters, {
        "parameter_storage": parameters.hex(" "), "response_storage": response.hex(" "),
        "state": state, "state_name": enum_names("CheckPSState").get(state, "unknown"),
        "irq_code_sum": irq_sum,
    }, "# The storage byte strings include alignment bytes after the buffers.\n")
    return Part("CD initial state", blob.offset(names.parameters), blob.offset(names.pattern_sizes),
                "tables/cd_state.yaml", content)


def read_pattern_sizes(blob: Blob[CheckpsSymbols]) -> Part:
    """The 17 width/height pairs used to draw the warning's red rings."""
    address = blob.symbols.pattern_sizes
    raw = read_bytes(blob, address, PATTERN_SIZE_COUNT * 2, "pattern sizes")
    sizes = [{"width": w, "height": h} for w, h in struct.iter_unpack("<BB", raw)]
    content = Table(address, {"sizes": sizes})
    start = blob.offset(address)
    return Part("pattern sizes", start, start + len(raw), "tables/pattern_sizes.yaml", content)


def read_digits(blob: Blob[CheckpsSymbols]) -> Part:
    """The Shift-JIS decimal and hexadecimal digits used by the cached font renderer."""
    tables = {}
    names = blob.symbols
    for label, first, last in (
        ("decimal", names.decimal_glyphs, names.hex_glyphs),
        ("hexadecimal", names.hex_glyphs, names.clut_prefix),
    ):
        raw = read_bytes(blob, first, last - first, label + " glyphs")
        if len(raw) % 2:
            raise ValueError(f"{label} glyph table has an odd byte count")
        glyphs = []
        for index in range(0, len(raw), 2):
            pair = raw[index:index + 2]
            if pair == b"\x00\x00":
                break
            glyphs.append(pair.decode("shift_jis"))
        else:
            raise ValueError(f"{label} glyph table has no zero terminator")
        tables[label] = {"address": hex_address(first), "glyphs": glyphs, "bytes": raw.hex(" ")}
    content = Table(names.decimal_glyphs, tables)
    return Part("digit glyphs", blob.offset(names.decimal_glyphs), blob.offset(names.clut_prefix),
                "tables/digit_glyphs.yaml", content)


def read_clut_prefix(blob: Blob[CheckpsSymbols]) -> Part:
    """Only the first six glyph colors are stored here; the rest comes from BSS."""
    address = blob.symbols.clut_prefix
    raw = read_bytes(blob, address, GLYPH_CLUT_PREFIX_BYTES, "glyph CLUT prefix")
    colors = [f"0x{value:04X}" for value, in struct.iter_unpack("<H", raw)]
    content = Table(address, {"colors": colors},
                    "# The renderer uploads 16 colors; the last ten come from adjacent zero-initialized BSS.\n")
    start = blob.offset(address)
    return Part("glyph CLUT prefix", start, start + len(raw), "tables/glyph_clut.yaml", content)


def read_rodata(blob: Blob[CheckpsSymbols]) -> list[Part]:
    """The Shift-JIS warning and the signs that reflect a ring into four quadrants."""
    address = blob.symbols.warning
    raw = read_bytes(blob, address, WARNING_BYTES, "hardware warning")
    text = raw.split(b"\x00", 1)[0].decode("shift_jis")
    warning = Table(address, {"text": text, "lines": text.splitlines(), "bytes": raw.hex(" ")})
    start = blob.offset(address)
    parts = [Part("hardware warning", start, start + len(raw), "text/warning.yaml", warning)]
    address = blob.symbols.quadrant_signs
    raw = read_bytes(blob, address, PATTERN_QUADRANT_COUNT * 2, "quadrant signs")
    signs = [{"x": x, "y": y} for x, y in struct.iter_unpack("<bb", raw)]
    start = blob.offset(address)
    content = Table(address, {"signs": signs})
    parts.append(Part("quadrant signs", start, start + len(raw), "tables/pattern_signs.yaml", content))
    return cover_gaps(blob, parts)


# ---------------------------------------------------------------------------
# Writing


@write_part.register
def _write_table(content: Table, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), **content.values}, content.note)


@write_part.register
def _write_audio_bank(content: AudioBank, path: Path) -> None:
    dump_yaml(path, {"address": hex_address(content.address), **content.metadata})
    (path.parent / "bank.akao").write_bytes(content.data)
    (path.parent / "samples.adpcm").write_bytes(content.samples)


@write_part.register
def _write_image(content: Image, path: Path) -> None:
    """Export the original TIM and one PNG for every stored 16-color palette."""
    path.parent.mkdir(parents=True, exist_ok=True)
    tim = content.tim
    (path.parent / "image.tim").write_bytes(tim.to_bytes())
    colors = [value for value, in struct.iter_unpack("<H", tim.clut.payload)]
    palettes = []
    for index in range(len(colors) // 16):
        palette = colors[index * 16 : (index + 1) * 16]
        rgba = [icon_set.bgr555_to_rgba(value) for value in palette]
        pixels = []
        for byte in tim.pixels.payload:
            pixels.extend((rgba[byte & 15], rgba[byte >> 4]))
        file = f"palette_{index:02X}.png"
        png.write_rgba(path.parent / file, tim.pixels.width_words * 4, tim.pixels.height, pixels)
        palettes.append({"index": index, "file": file, "colors": [f"0x{c:04X}" for c in palette]})
    dump_yaml(path, {
        "address": hex_address(content.address), "tim": "image.tim",
        "width": tim.pixels.width_words * 4, "height": tim.pixels.height,
        "stored_layout": tim.metadata(), "palettes": palettes,
        "note": "Each PNG shows the stored texture through one palette, not the animated screen. "
                "CHECKPS uploads the pixels at (320, 0) and flattens the CLUT at (0, 480). "
                "Palette value zero is transparent; other colors are shown opaque.",
    })


# ---------------------------------------------------------------------------
# Putting it together


def load_blobs(inputs: Inputs) -> tuple[Blob[CheckpsSymbols], Blob[CheckpsSymbols]]:
    """Find the initialized data and warning rodata through the splat config."""
    names = CheckpsSymbols.load(inputs.symbol_file)
    files = splat_config.data_files(inputs.overlay_config, inputs.assets)
    blobs = []
    for address, what in ((names.audio, "embedded AKAO"), (names.warning, "hardware warning")):
        source = splat_config.file_containing(files, address, what)
        if not source.path.exists():
            raise ValueError(f"{source.path} is missing; run make splat first")
        data = source.path.read_bytes()
        if len(data) != source.end - source.start:
            raise ValueError(f"{source.path} size does not match the splat config; run make splat again")
        blobs.append(Blob(data, source.start, source.path.name, inputs.version, names))
    data, rodata = blobs
    if data.address + len(data.data) != names.bss:
        raise ValueError("CHECKPS data blob must end where BSS starts")
    return data, rodata


def extract(inputs: Inputs, output: Path) -> None:
    """Read everything first, then move the completed export folder into place."""
    if output.exists():
        raise FileExistsError(f"{output} already exists")
    blob, rodata = load_blobs(inputs)
    parts, rodata_parts = read_blob(blob), read_rodata(rodata)
    byte_map = {
        "source": blob.file_name, "address": hex_address(blob.address), "size": f"0x{len(blob.data):X}",
        "ranges": [byte_map_entry(blob, part) for part in parts],
        "other_files": [{
            "source": rodata.file_name, "address": hex_address(rodata.address), "size": f"0x{len(rodata.data):X}",
            "ranges": [byte_map_entry(rodata, part) for part in rodata_parts],
        }],
        "bss": {"address": hex_address(blob.symbols.bss),
                "note": "Runtime buffers remain in the overlay's BSS sections and are outside this data blob."},
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix=f".{output.name}-", dir=output.parent))
    try:
        for part in parts + rodata_parts:
            if part.content is not None:
                write_part(part.content, staging / part.file)
        dump_yaml(staging / "byte-map.yaml", byte_map)
        staging.rename(output)
    except BaseException:
        shutil.rmtree(staging, ignore_errors=True)
        raise


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("output", type=Path, help="new folder to write")
    parser.add_argument("--version", choices=("us", "jp"), default="us")
    parser.add_argument("--config", type=Path, help="config folder (default config/<version>)")
    parser.add_argument("--assets", type=Path, help="splat assets (default assets/<version>)")
    args = parser.parse_args()
    inputs = Inputs(args.version, args.config or REPO_ROOT / "config" / args.version,
                    args.assets or REPO_ROOT / "assets" / args.version)
    try:
        extract(inputs, args.output)
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

"""Decode scene text tables without loading another game file.

field_text_window_api.c indexes u16 offsets relative to the string section.
field_text.c supplies the control widths. US dictionary phrases are shared
with the existing GNAME reader; other glyphs remain explicit byte tokens.
"""

from __future__ import annotations

import struct

from tools.data.formats.name_entry_resource import DICTIONARY_TOKENS
from tools.data.scenes.scene_format import SceneHeader
from tools.data.scenes.section_cli import run


CONTROLS = (
    'end', 'newline', 'wait_newline', 'wait_clear', 'clear', 'wait', 'finish',
    'choice', 'two_spaces', 'three_spaces', 'four_spaces', 'spaces',
    'short_delay', 'delay', 'macro', 'inline_text', 'color', 'default_color',
    'prefixed_glyph_run', 'indent',
)
ARGUMENT_CONTROLS = frozenset((11, 13, 14, 16))


def uncovered(data: bytes, spans: list[tuple[int, int]], offset: int) -> list[dict]:
    """Describe bytes outside decoded ranges, counting shared ranges once."""
    result = []
    cursor = 0
    for start, end in sorted(spans + [(len(data), len(data))]):
        if start > cursor:
            result.append({'offset': offset + cursor, 'size': start - cursor,
                           'bytes': data[cursor:start].hex(' ')})
        cursor = max(cursor, end)
    return result


def read_text(data: bytes, start: int = 0, encoding: str = 'us') -> tuple[str, list[dict], int, bool]:
    """Read tokens through end (0) or finish (6); zero-valued command arguments are not terminators.

    The returned end is exclusive. A fixed-width name need not have an end code;
    table entries record missing terminators as diagnostics.
    """
    if encoding not in ('us', 'jp'):
        raise ValueError(f'unknown text encoding: {encoding}')
    parts, tokens = [], []
    cursor = start
    while cursor < len(data):
        code = data[cursor]
        width = 1
        if code in ARGUMENT_CONTROLS or code == 25 or code == 31 or (encoding == 'jp' and 25 <= code <= 31):
            width = 2
        elif code == 18:
            width = 2 if encoding == 'jp' else 3
        raw = data[cursor:cursor + width]
        if len(raw) != width:
            tokens.append({'relative_offset': cursor - start, 'kind': 'truncated', 'bytes': raw.hex(' ')})
            parts.append('{truncated ' + raw.hex(' ').upper() + '}')
            return ''.join(parts), tokens, len(data), False
        token = {'relative_offset': cursor - start, 'bytes': raw.hex(' ')}
        if code == 0:
            tokens.append({**token, 'kind': 'end'})
            return ''.join(parts), tokens, cursor + 1, True
        if 32 <= code < 127:
            token.update(kind='character', text=chr(code))
        elif encoding == 'us' and raw in DICTIONARY_TOKENS:
            token.update(kind='dictionary', text=DICTIONARY_TOKENS[raw])
        elif code < len(CONTROLS):
            token.update(kind='control', command=CONTROLS[code])
            if width > 1:
                token['arguments'] = list(raw[1:])
            label = CONTROLS[code] + (':' + ','.join(str(value) for value in raw[1:]) if width > 1 else '')
            token['text'] = '\n' if code == 1 else '{' + label + '}'
        else:
            token.update(kind='glyph', text='{' + raw.hex(' ').upper() + '}')
        if token['kind'] == 'character' and tokens and tokens[-1]['kind'] == 'character':
            tokens[-1]['text'] += token['text']
            tokens[-1]['bytes'] += ' ' + token['bytes']
        else:
            tokens.append(token)
        parts.append(token['text'])
        if code == 6:
            return ''.join(parts), tokens, cursor + width, True
        cursor += width
    return ''.join(parts), tokens, cursor, False


def text_preview(raw: bytes, encoding: str = 'us') -> str:
    return read_text(raw, encoding=encoding)[0]


def read_text_table(data: bytes, offset: int, encoding: str = 'us') -> dict:
    """Preserve directory order, aliases, interior pointers and decoding gaps."""
    result = {'offset': offset, 'size': len(data), 'encoding': encoding,
              'pointer_base': offset, 'directory_size': 0, 'entries': [], 'strings': [],
              'diagnostics': [], 'undecoded': []}
    if not data:
        return result
    if len(data) < 2:
        raise ValueError('truncated string directory')
    first = struct.unpack_from('<H', data)[0]
    # A zero header occurs in empty placeholder sections; no entry count is implied.
    if first == 0:
        result['diagnostics'].append('Zero first offset: no string directory can be established.')
        result['undecoded'] = uncovered(data, [], offset)
        return result
    if first % 2 or first > len(data):
        raise ValueError('invalid string directory size')
    offsets = struct.unpack_from(f'<{first // 2}H', data)
    result['directory_size'] = first
    spans = [(0, first)]
    decoded = {}
    for index, start in enumerate(offsets):
        entry = {'index': index, 'relative_offset': start, 'offset': offset + start}
        result['entries'].append(entry)
        if not first <= start < len(data):
            entry['unresolved'] = 'String offset is outside the text payload.'
            continue
        if start not in decoded:
            text, tokens, end, terminated = read_text(data, start, encoding)
            decoded[start] = len(result['strings'])
            result['strings'].append({'index': decoded[start], 'offset': offset + start,
                                      'size': end - start, 'text': text, 'terminated': terminated,
                                      'tokens': tokens, 'bytes': data[start:end].hex(' ')})
            spans.append((start, end))
            if not terminated:
                result['diagnostics'].append(f'String at 0x{offset + start:X} has no complete end/finish terminator.')
        entry['string_index'] = decoded[start]
    result['undecoded'] = uncovered(data, spans, offset)
    return result


def read_strings(data: bytes, header: SceneHeader, *, text_encoding: str = 'us') -> dict:
    return read_text_table(data[header.strings:header.actor_scripts], header.strings, text_encoding)


def format_strings(document: dict) -> str:
    lines = ['STRINGS', f"IMG offset 0x{document['offset']:08X}; {document['size']} bytes; encoding={document['encoding']}",
             'Directory offsets are relative to the text table. Indices are zero-based.',
             'Braces show controls or unmapped glyph bytes; macros remain unexpanded.',
             'Tokens and original bytes are recorded in YAML.', '']
    for entry in document['entries']:
        lines.append(f"[{entry['index']:03d}] IMG 0x{entry['offset']:08X}")
        if 'unresolved' in entry:
            lines.append('  ' + entry['unresolved'])
        else:
            string = document['strings'][entry['string_index']]
            lines.extend('  ' + line for line in (string['text'] or '(empty string)').split('\n'))
            if not string['terminated']:
                lines.append('  WARNING: missing terminator')
        lines.append('')
    for diagnostic in document['diagnostics']:
        lines.append('Diagnostic: ' + diagnostic)
    for span in document['undecoded']:
        lines.append(f"Unreferenced bytes at 0x{span['offset']:08X}: {span['bytes']}")
    return '\n'.join(lines) + '\n'


def main(argv: list[str] | None = None) -> int:
    return run(read_strings, __doc__, argv, kind='strings', text_renderer=format_strings, text_encoding=True)


if __name__ == '__main__':
    raise SystemExit(main())

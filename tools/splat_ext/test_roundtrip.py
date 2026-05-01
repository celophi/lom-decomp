from compress import compress
from decompress import decompress
import random

random.seed(42)

tests = [
    (bytes([0x05] * 20), 'repeated nibble byte (F0)'),
    (bytes([0xAB] * 20), 'repeated non-nibble byte (F1)'),
    (bytes(range(10, 50)), 'ascending run (F8)'),
    (bytes(range(50, 10, -1)), 'descending run (F9)'),
    (bytes([0x03, 0x07] * 10), '2-byte nibble pairs (F2)'),
    (bytes([0xAB, 0xCD] * 10), '2-byte repeat (F3)'),
    (bytes([0x11, 0x22, 0x33] * 8), '3-byte repeat (F4)'),
    (bytes([(i * 3) & 0xFF for i in range(50)]), 'arith step (FA)'),
    (bytes([random.randint(0, 30) for _ in range(500)]), 'random low'),
    (bytes([random.randint(0, 255) for _ in range(1000)]), 'random full'),
]

# F5: fixed byte alternating with varying bytes
f5_data = bytearray()
for i in range(20):
    f5_data.append(0x42)
    f5_data.append(i & 0xFF)
tests.append((bytes(f5_data), 'fixed+varying pairs (F5)'))

# F6: 2 fixed bytes alternating with varying
f6_data = bytearray()
for i in range(15):
    f6_data.append(0x11)
    f6_data.append(0x22)
    f6_data.append(i & 0xFF)
tests.append((bytes(f6_data), '2fixed+varying triplets (F6)'))

# F7: 3 fixed bytes alternating with varying
f7_data = bytearray()
for i in range(10):
    f7_data.append(0xAA)
    f7_data.append(0xBB)
    f7_data.append(0xCC)
    f7_data.append(i & 0xFF)
tests.append((bytes(f7_data), '3fixed+varying quads (F7)'))

# FB: 16-bit pair run with delta
pairs = bytearray()
val = 0x1000
for _ in range(10):
    pairs.append(val & 0xFF)
    pairs.append((val >> 8) & 0xFF)
    val = (val + 5) & 0xFFFF
tests.append((bytes(pairs), '16-bit pair run (FB)'))

# FC: back-reference (long repeated pattern)
base = bytes([random.randint(0, 255) for _ in range(50)])
fc_data = base + base + base
tests.append((fc_data, 'back-reference repeat (FC/FD)'))

all_pass = True
for data, name in tests:
    c = compress(data)
    d = bytes(decompress(c))
    ok = d == data
    status = 'PASS' if ok else 'FAIL'
    print(f'{status}: {name}: {len(data)} -> {len(c)} bytes')
    if not ok:
        all_pass = False
        print(f'  Expected: {data[:20].hex()}...')
        print(f'  Got:      {d[:20].hex()}...')

print()
print('All tests passed!' if all_pass else 'SOME TESTS FAILED!')

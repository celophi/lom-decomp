"""Verify simple_compressor round-trips perfectly and report ratios."""
import os, sys
sys.path.insert(0, 'd:/tmp')
from simple_compressor import compress
from decompressor import decompress

files = ['ADDHERO','CARDA','CHECKPS','CLOAD','FIELD','GNAME','GOLEM','GOSUB',
         'GOVER','MENU','MOVIE','NIKI','SHOP','TITLE','ZUKAN']

print(f"{'File':10s} {'Inflated':>10s} {'Compressed':>11s} {'Ratio':>7s} {'Ref':>10s} {'vs Ref':>8s} {'Match?':>8s}")
print('-' * 75)

ok_count = 0
for name in files:
    inf = f'd:/tmp/decompressed/{name}.BIN.decompressed'
    ref = f'd:/tmp/compressed/{name}.BIN'
    if not os.path.exists(inf):
        continue
    data = open(inf,'rb').read()
    comp = compress(data)
    decomp = decompress(comp)
    matches = bytes(decomp) == data
    if matches:
        ok_count += 1
    ref_size = os.path.getsize(ref) if os.path.exists(ref) else 0
    ratio = len(comp)/len(data) if data else 0
    vs_ref = f'{(len(comp)/ref_size - 1)*100:+.1f}%' if ref_size else '-'
    status = 'OK' if matches else 'FAIL'
    print(f'{name:10s} {len(data):10,d} {len(comp):11,d} {ratio:7.3f} {ref_size:10,d} {vs_ref:>8s} {status:>8s}')

print(f'\n{ok_count}/{len(files)} files round-trip correctly.')

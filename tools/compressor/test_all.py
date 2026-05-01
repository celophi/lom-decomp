import subprocess, os, sys

files = ['ADDHERO','CARDA','CHECKPS','CLOAD','FIELD','GNAME','GOLEM','GOSUB','GOVER','MENU','MOVIE','NIKI','SHOP','TITLE','ZUKAN']
results = []

for name in files:
    inf = f'd:/tmp/decompressed/{name}.BIN.decompressed'
    ref = f'd:/tmp/compressed/{name}.BIN'
    out = f'd:/tmp/{name}.out'
    if not os.path.exists(inf) or not os.path.exists(ref):
        continue
    r = subprocess.run(['python','compressor.py',inf,out], capture_output=True, cwd='d:/tmp')
    a = open(ref,'rb').read()
    b = open(out,'rb').read()
    diffs = sum(1 for x,y in zip(a,b) if x!=y) + abs(len(a)-len(b))
    first = next((i for i,(x,y) in enumerate(zip(a,b)) if x!=y), None)
    status = 'OK' if diffs == 0 else f'{diffs} diffs (first={first})'
    print(f'{name:10s}: {status}')
    results.append((name, diffs))

ok = sum(1 for _,d in results if d==0)
print(f'\n{ok}/{len(results)} files match exactly')

#!/usr/bin/env python3
import zipfile, os, hashlib

p = r'C:\Users\mirot\.openclaw\workspace\ChameleonUltra\ultra-dfu-full.zip'
needles = [b'DESFire EV1', b'DESFire EV2', b'T5577', b'EM4305', b'FDX-B', b'Indala', b'Keri', b'Paradox', b'PAC']

print('=== Chameleon Ultra DFU Package v2.2 ===')
print(f'ZIP Size: {os.path.getsize(p)} bytes')

with zipfile.ZipFile(p) as z:
    app = z.read('application.bin')
    print(f'APP Size: {len(app)} bytes')
    
    print('\nFeature Strings:')
    for n in needles:
        cnt = app.lower().count(n.lower())
        print(f'  {n.decode():12} count={cnt}')

print('\n=== Verification Complete ===')

#!/usr/bin/env python3
"""
Convert raw 32-bit little-endian framebuffer dump to PNG.
Usage: python3 tools/fb_to_png.py input.bin output.png width height
"""
import sys
from PIL import Image

if len(sys.argv) < 5:
    print("Usage: fb_to_png.py input.bin output.png width height")
    sys.exit(2)

infile, outfile = sys.argv[1], sys.argv[2]
width = int(sys.argv[3])
height = int(sys.argv[4])

with open(infile, 'rb') as f:
    data = f.read()

# Expect width*height*4 bytes
expected = width * height * 4
if len(data) < expected:
    print(f"Warning: input shorter than expected ({len(data)} < {expected})")

# Interpret as little-endian BGRA or ARGB? We wrote 0x00RRGGBB; so bytes are BB GG RR 00
# We'll read as BGRA and convert to RGB
img = Image.frombytes('RGBA', (width, height), data, 'raw', 'BGRA')
img.convert('RGB').save(outfile)
print(f"Wrote {outfile}")

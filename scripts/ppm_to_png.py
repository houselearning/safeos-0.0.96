#!/usr/bin/env python3
import sys
from PIL import Image

if len(sys.argv) < 3:
    print("Usage: ppm_to_png.py input.ppm output.png")
    sys.exit(2)

infile, outfile = sys.argv[1], sys.argv[2]
img = Image.open(infile)
img.save(outfile)
print("Wrote", outfile)

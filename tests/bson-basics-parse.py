#!/usr/bin/python3

import bson
import sys

#msvcrt.setmode (sys.stdin.fileno(), os.O_BINARY)
bs = sys.stdin.buffer.read(51)

b = bson.loads(bs)

print(b["y"])
print(b["i"])
print(b["p"].decode('ascii'))

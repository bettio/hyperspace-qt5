#!/usr/bin/python3

import bson
import sys

b = {}
b["y"] = 42
b["i"] = "the things"
b["p"] = b"binary things"

sys.stdout.buffer.write(bson.dumps(b))

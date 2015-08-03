#!/usr/bin/python3

import sys

def postprocess():
    # Append bytes until file is 8192 bytes long (16 sectors)
    f = open(sys.argv[1], "ab+")
    if f.tell() > 8192:
        print(sys.argv[0], ": file too big!", file=sys.stderr)
        exit(1)
    while f.tell() < 8192:
        f.write(b'\0')

if __name__ == "__main__":
    postprocess()

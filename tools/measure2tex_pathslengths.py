#!/usr/bin/env python3

import sys


def parse_input(f):
    # lines: <stage>, <genStage level within this stage>, <path length>

    r = {}
    for line in f:
        line = line.strip()
        if line:  # empty string is evaluated as false
            s, rec_level, l = tuple(map(int, line.split()))
            if s not in r:
                r[s] = {}

            if rec_level not in r[s]:
                r[s][rec_level] = []

            if l != 0:  # don't count empty "paths" (when bond is created)
                r[s][rec_level].append(l)
    return r


if __name__ == "__main__":
    if len(sys.argv) != 2 or sys.argv[1] == "-h":
        print("Usage: {} <file>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    # Open file
    try:
        f = open(sys.argv[1], "r")
    except:
        print("File '{}' not found!".format(sys.argv[1]), file=sys.stderr)

    # Read into a dict
    data = parse_input(f)

    for s in data:
        for level in data[s]:
            avg = sum(data[s][level]) / len(data[s][level])
            print("{} {} {}".format(s, level, avg))

    # Bye
    f.close()

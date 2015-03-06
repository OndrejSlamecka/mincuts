#!/usr/bin/env python3

# Usage: ./tools/names2ids.py data/zlin-silnice.csv < ../closuresim/results/results-all.csv

import sys

def createNameTable(filename):
    table = {}
    with open(filename) as f:
        for line in f:
            s = line.split(";")
            id, u, v, name = s[0], s[1], s[2], s[3]
            table[name] = (id,u,v)
    return table

def names2ids(table):
    for line in sys.stdin:
        s = line.split(";")
        for i in range(len(s)-3):
            id, u, v = table[s[i]]
            #print(id + "(" + u + "," + v + ")", end="")
            print(id, end="")
            if i != len(s) - 4:
                print(",", end="")
        print("")

def main():
    if len(sys.argv) < 2:
        print("Usage: $ ./names2ids.py roadsFile")
        sys.exit()

    table = createNameTable(sys.argv[1])
    names2ids(table)

if __name__ == "__main__":
    main()

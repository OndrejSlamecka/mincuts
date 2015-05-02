#!/usr/bin/env python3
import sys

### Parsing graph6 format
# Graph6 parser is from the NetworkX project which is governed by the BSD license
# For full license info see https://networkx.github.io/documentation/latest/reference/legal.html
# https://networkx.github.io/documentation/latest/_modules/networkx/readwrite/graph6.html#read_graph6
def parse_graph6(string):
    def bits():
        """Return sequence of individual bits from 6-bit-per-value
        list of data values."""
        for d in data:
            for i in [5,4,3,2,1,0]:
                yield (d>>i)&1

    data = graph6_to_data(string)
    n, data = data_to_n(data)
    nd = (n*(n-1)//2 + 5) // 6

    g = [ [0 for j in range(n)] for i in range(n) ]
    for (i,j),b in zip([(i,j) for j in range(1,n) for i in range(j)], bits()):
        if b:
            g[i][j] = 1
            g[j][i] = 1

    return g

def graph6_to_data(string):
    """Convert graph6 character sequence to 6-bit integers."""
    v = [ord(c)-63 for c in string]
    if len(v) > 0 and (min(v) < 0 or max(v) > 63):
        return None
    return v

def data_to_n(data):
    """Read initial one-, four- or eight-unit value from graph6
    integer sequence.

    Return (value, rest of seq.)"""
    if data[0] <= 62:
        return data[0], data[1:]
    if data[1] <= 62:
        return (data[1]<<12) + (data[2]<<6) + data[3], data[4:]
    return ((data[2]<<30) + (data[3]<<24) + (data[4]<<18) +
            (data[5]<<12) + (data[6]<<6) + data[7], data[8:])


### Do the job

def graph2csv(g):
    n = len(g)
    r = ""

    k = 0
    for i in range(n):
        for j in range(i + 1, n):
            r += "{0};{1};{2}\n".format(k, i, j)
            k += 1

    return r.rstrip() # strip the trailing \n

if __name__ == "__main__":
    if (len(sys.argv) == 2 and sys.argv[1] == "-h") or len(sys.argv) > 2:
        print("Reads graph in graph6 format and outputs it in CSV.\n"\
              "First argument (if provided and other than -h) is used as the"\
              " output.", file=sys.stderr)
        sys.exit(1)

    line = input().rstrip()
    g = parse_graph6(line)
    csv = graph2csv(g)

    if len(sys.argv) == 2:
        f = open(sys.argv[1], "w+")
        print(csv, file=f)
        f.close()
    else:
        print(csv)

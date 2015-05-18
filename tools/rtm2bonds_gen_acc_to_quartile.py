#!/usr/bin/env python3

import sys
from numpy import percentile, linspace, set_printoptions

def parse_input(f, allow_zeroduration_and_nonempty):
    r = []
    for line in f:
        line = line.strip()
        if line: # empty string is evaluated as false
            v = tuple(map(int, line.split()[1:]))
            if allow_zeroduration_and_nonempty or (v[0] != 0 or v[1] != 0):
                r.append(v)
    return r

def make_table(records):
    # Duration is in the first column and Python defaults to sort by the first column
    records = sorted(records)

    durations = [record[0] for record in records]

    # Sum # of bonds produced in each quarter (quarter wrt. time lengths)
    parts = 3 + 1 # the number of quantiles plus one
    boundaries_indicies = [p for p in linspace(0,1,parts+1)]
    boundaries = [percentile(durations, p*100) for p in boundaries_indicies]

    sums = [0] * parts
    counts = [0] * parts    
    q = 0 # which quarter we're in

    # Go through records, keep track of what quarter we're in
    for r in records:
        # If this r belongs to a different quarter, increment q
        while r[0] > boundaries[q + 1]:
            q += 1

        sums[q] += r[1]
        counts[q] += 1

    # Create a neat table for TeX
    for i in range(parts):
        print(("%g" % boundaries_indicies[i]) + "\t&\t" + ("%g" % boundaries_indicies[i+1]), end="\t& ")
        print(("%g" % boundaries[i]         ) + "\t&\t" + ("%g" % boundaries[i+1]), end="\t& ")
        print(str(counts[i] ), end="\t& ")
        print(str(sums[i]), end=' \\\\\n')

def usage():
    print("Usage:\t" + sys.argv[0] + " <path to data file> [-a]\n\n" \
          "\t-a -- include those records which have zero time and zero bonds\n" \
          "\n\tSplits the timeline of given rtm file by quartiles " \
          "\n\tand reports sums of # of bonds in each quarter.",
          file=sys.stderr)

if __name__ == "__main__":
    if len(sys.argv) < 2 or len(sys.argv) > 3 or (len(sys.argv) == 3 and sys.argv[2] != "-a"):
        usage()
        sys.exit(1)

    allow_zeroduration_and_empty = False
    if len(sys.argv) == 3:
        allow_zeroduration_and_empty = True

    try:
        f = open(sys.argv[1])
        records = parse_input(f, allow_zeroduration_and_empty)
        make_table(records)
    except OSError as e: # Catches just error when openning the file
        print("I/O error ({0}): {1}".format(e.errno, e.strerror))



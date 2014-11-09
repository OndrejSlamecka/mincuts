#!/usr/bin/env python3

import sys

def createNameTable(filename):
	table = {}
	with open(filename) as f:
		for line in f:
			s = line.split(";")
			id = s[0]
			u  = s[1]
			v = s[2]
			name = s[3]
			table[name] = (id,u,v)
	return table

def names2ids(table):
	for line in sys.stdin:
		s = line.split(";")
		for i in range(len(s)-2):
			id, u, v = table[s[i]]
			print(id + "(" + u + "," + v + ")", end="")
			if i != len(s) - 3:
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


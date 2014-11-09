#!/usr/bin/env python3

# Works only with cuts of size two

import sys, re

def main():
	comparable = {}
	for line in sys.stdin:
		r = re.match('([\d]+)\([^)]*\),([\d]+)', line)
		e1, e2 = r.groups()
		comparable[e1 + '_' + e2] = (e2 + '_' + e1, line)

	seen = {k: False for k in comparable}
	for i in comparable:
		perm_id, line = comparable[i]
		seen[perm_id] = True
		if not seen[i]:
			print(line, end="")


if __name__ == "__main__":
	main()




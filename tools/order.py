#!/usr/bin/env python3

# TODO: Rewrite to not use cmp_tokey
# Works only with cuts of size two

import sys, re
from functools import cmp_to_key

def compare(l1,l2):
	if l1[0] != l2[0]:
		return l1[0] - l2[0]
	else:
		return l1[1] - l2[1]

def main():
	comparable = []		
	for line in sys.stdin:
		r = re.match('([\d]+)\([^)]*\),([\d]+)', line)
		e1, e2 = r.groups()
		comparable.append((int(e1), int(e2), line))

	s = sorted(comparable, key=cmp_to_key(compare))
	for line in s:
		print(line[2], end="")
	

if __name__ == "__main__":
    main()

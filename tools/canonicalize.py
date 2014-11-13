#!/usr/bin/env python3

# Works only with cuts of size two

import sys, re
from collections import OrderedDict

def main():
	lines= {}
	for line in sys.stdin:
		r = re.match('([\d]+)\(([^)]+)\),([\d]+)\(([^)]+)\)', line)
		e1, e1d, e2, e2d = r.groups()
	
		if e1 < e2:
			primary = e1
			primary_desc = e1d
			secondary = e2
			secondary_desc = e2d
		else:
			primary = e2
			primary_desc = e2d
			secondary = e1
			secondary_desc = e1d
		
		if not primary in lines:
			lines[primary] = {}

		l = primary + '(' + primary_desc + '),' + secondary + '(' + secondary_desc + ')'
		lines[primary][secondary] = l

	o = OrderedDict(sorted(lines.items(), key=lambda t: int(t[0])))
	for i in o:
		os = OrderedDict(sorted(o[i].items(), key=lambda t: int(t[0])))
		for j in os:	
			print(os[j])


if __name__ == "__main__":
	main()




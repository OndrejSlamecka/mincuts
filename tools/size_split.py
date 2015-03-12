#!/usr/bin/env python3

import sys, os

# TODO: Add help

def size_split(source, dest):
	def edges(line):
		n = 0
		p = False # inside parentheses
		for i in range(len(line)):
			if line[i] == '(':
				p = True
			elif line[i] == ')':
				p = False
			elif line[i] == ',' and not p:
				n += 1
		return n

	# File object cache
	files = {}
	def getfile(n):
		if not n in files:
			files[n] = open(dest + "/" + str(n), "w+")

		return files[n]

	# The split itself
	with open(source) as f:
		for line in f:
			n = edges(line) + 1
			f = getfile(n)
			print(line, file=f, end="")

	for i in range(len(files)):
		files[i+1].close()

def main():
	if len(sys.argv) < 2:
		print("Usage: $ ./size_split.py <file to split>")
		sys.exit()

	folder = sys.argv[1] + "bySize"

	if not os.path.exists(folder):
		os.makedirs(folder)

	size_split(sys.argv[1], folder)


if __name__ == "__main__":
	main()

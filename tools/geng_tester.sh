#!/bin/bash

command -v geng >/dev/null 2>&1 || { echo >&2 "\`geng\` from nauty" \
	   "(http://cs.anu.edu.au/~bdm/nauty/) is required for this program to run.";
	exit 1; }

if [[ $# -lt 2 || $# -gt 3 ]]; then
	echo "Usage: $0 <cut size bound> <# of components> [<# of proc. cores to use>]"
	echo -e "\t <# of components> can be exact or range (e.g. 2-3)"
	exit 1
fi

threads=""
if [[ $# -eq 3 ]]; then
	threads="-t$3"
fi

for ((i=2; i<= 32; i++)); do
	echo "Generating graphs on $i nodes"
	geng -cq "$i" | ./bin/tester "$1" "$2" "$threads"
done


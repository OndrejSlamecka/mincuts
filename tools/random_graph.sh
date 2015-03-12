#!/bin/bash

# TODO: Help

NODES=$1
EDGE_PROB=$2

command -v genrang >/dev/null 2>&1 || { echo >&2 "\`genrang\` from nauty" \
   "(http://cs.anu.edu.au/~bdm/nauty/) is required for this program to run.";
	exit 1; }

RG=$(genrang $NODES 1 -P$EDGE_PROB | \
	showg -q -e -l0  | \
	awk 'NR%2==0 {n=split($0,a,"  ");
		for (i=1; i<=n; i++) print i-1 " " a[i]}' | \
	sed 's/ /;/g')

echo "$RG"


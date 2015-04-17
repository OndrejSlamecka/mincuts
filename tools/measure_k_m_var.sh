#!/bin/bash

if [[ $# -ne 3 ]]; then
	echo "Usage: $0 <edge_file.csv> <max cut size bound> <max # of components>"
	echo "Outputs space separated triplet <# of components> <# of edges> <time in seconds>"
	exit 1
fi

EDGES=$2
COMPONENTS=$3


for c in $(seq 2 $COMPONENTS); do # seq is GNU only...
	for e in $(seq $c $EDGES); do
		echo "# Starting mincuts to generate $c-bonds with maximum $e edges "
		t=$(/usr/bin/time -f "%U" mincuts $1 $e $c 2>&1 1>/dev/null)
		echo $c $e $t
	done
done


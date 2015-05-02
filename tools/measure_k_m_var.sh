#!/usr/bin/env bash

if [[ $# -ne 3 ]]; then
	echo "Usage: $0 <edge_file.csv> <min>-<max cut size bound> <min>-<max # of components>"
	echo "Outputs space separated tuple: <# of components> <# of edges> <time in seconds> <# of bonds>"
	exit 1
fi

# Parse input
min_e=1
max_e=$2
if [[ "$2" == *"-"* ]]; then
	parts=(${2//-/ })
	min_e=${parts[0]}
	max_e=${parts[1]}
fi

min_c=2
max_c=$3
if [[ "$3" == *"-"* ]]; then
	parts=(${3//-/ })
	min_c=${parts[0]}
	max_c=${parts[1]}
fi

# Run
for ((c=$min_c; c <= $max_c; c++)); do
	cm1=$(($c - 1))
	edges_start=$(( $cm1 > $min_e ? $cm1 : $min_e))
	for ((e=$edges_start; e <= $max_e; e++)); do
		echo "# Running mincuts: $c-bonds with at max $e edges (`date`)"
		exec 3>&2
		t=$( { { /usr/bin/time -f "%U" ./bin/mincuts "$1" "$e" "$c" 2>&3; } | wc -l; } 2>&1 3>&1 )
		echo "$c $e" ${t[0]} ${t[1]}
		exec 3>&-
	done
done


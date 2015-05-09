#!/usr/bin/env bash

if [[ $# -ne 3 ]]; then
	echo "Usage: $0 <edge file.csv> <max cut size bound> <max # of component>"
	exit 1
fi

min_c=3
min_e=3

max_e=$2
max_c=$3

for ((c=$min_c; c <= $max_c; c++)); do
	edges_start=$(( $c > $min_e ? $c : $min_e ))
	for ((e=$edges_start; e <= $max_e; e++)); do
		echo "# Running mincuts: $c-bonds with at max $e edges (`date`)"
		./bin/mincuts "$1" "$e" "$c" > tmp/measure_noncan_storage.csv
		noncan=$(wc -l tmp/measure_noncan_storage.csv | awk '{print $1}')
		can=$(./bin/cutuniq tmp/measure_noncan_storage.csv | wc -l | awk '{print $1}')
		echo "$c $e $noncan $can"
	done
done

rm tmp/measure_noncan_storage.csv

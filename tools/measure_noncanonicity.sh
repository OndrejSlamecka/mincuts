#!/usr/bin/env bash

if [[ $# -ne 3 ]]; then
	echo "Usage: $0 <edge file.csv> <min>-<max cut size bound> <min>-<max # of components>"
	exit 1
fi

min_e=3
max_e=$2
if [[ "$2" == *"-"* ]]; then
	parts=(${2//-/ })
	min_e=${parts[0]}
	max_e=${parts[1]}
fi

min_c=3
max_c=$3
if [[ "$3" == *"-"* ]]; then
	parts=(${3//-/ })
	min_c=${parts[0]}
	max_c=${parts[1]}
fi

storage_file=tmp/measure_noncan_storage_$(basename $1)

for ((c=$min_c; c <= $max_c; c++)); do
	edges_start=$(( $c > $min_e ? $c : $min_e ))
	for ((e=$edges_start; e <= $max_e; e++)); do
		echo "# Running mincuts: $c-bonds with at max $e edges (`date`)"
		./bin/mincuts "$1" "$e" "$c" > "$storage_file"
		noncan=$(wc -l "$storage_file" | awk '{print $1}')
		can=$(./bin/cutuniq $storage_file | wc -l | awk '{print $1}')
		echo "$c $e $noncan $can"
	done
done

rm $storage_file


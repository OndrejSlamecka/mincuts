#!/bin/bash

if [[ $# -ne 0 ]]; then
	echo "Input: A file produced by measure_on_graph.sh tool"
	echo "Output: TeX table of the result"
	exit 1
fi

# Process the input
components=2
edges=2
declare -A table

while read line
do
	# Save the biggest # of components and # of edges
	if [[ $line != "#"* && $line != "" ]]; then
		split_line=($line)
		c=${split_line[0]}
		e=${split_line[1]}
		table[$c,$e]=${split_line[2]}

		if [ "$c" -gt "$components" ]; then
			components=$c
		fi
		if [ "$e" -gt "$edges" ]; then
			edges=$e
		fi
	fi
done


function cell {
	printf "%10s " $1
}

function cellend {
	if [[ $1 != $2 ]]; then
		printf "&"
	else
		printf '\\\\'	
	fi
}

# Table header
printf "            &"
for ((j=2;j<=edges;j++)) do
	cell $j
	cellend $j $edges
done
echo ""

# Contents
for ((i=2;i<=components;i++)) do
	cell $i
	printf " &"
	for ((j=2;j<=edges;j++)) do
		cell ${table[$i,$j]}
		cellend $j $edges
	done
	echo ""
done

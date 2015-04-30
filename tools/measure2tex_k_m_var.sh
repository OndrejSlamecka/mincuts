#!/bin/bash

if [[ $# -ne 0 ]] && [[ $1 != "-c" ]]; then
	echo "Input: A file produced by measure_on_graph.sh tool, add -c switch " \
		 "to output number of bonds produced instead of time taken"
	echo "Output: TeX table of the result"
	exit 1
fi

argv0=$1

# Process the input
components=2
edges=1
declare -A table

while read line
do
	# Save the biggest # of components and # of edges
	if [[ "$line" != "#"* && "$line" != "" ]]; then
		split_line=($line)
		c=${split_line[0]}
		e=${split_line[1]}
		if [[ $argv0 == "-c" ]]; then
			table[$c,$e]=${split_line[3]}
		else
			table[$c,$e]=${split_line[2]}
		fi

		if [[ $c -gt "$components" ]]; then
			components=$c
		fi
		if [[ $e -gt "$edges" ]]; then
			edges=$e
		fi
	fi
done


function cell {
	printf "%10s " $1
}

function cellend {
	if [[ $1 != "$2" ]]; then
		printf "&"
	else
		printf '\\\\'
	fi
}

# Table header
printf "            &"
for ((j=1;j<=edges;j++)) do
	cell "$j"
	cellend "$j" "$edges"
done
echo ""

# Contents
for ((i=2; i<=components; i++)) do
	cell $i
	printf " &"
	for ((j=1; j<=edges; j++)) do
		cell "${table[$i,$j]}"
		cellend "$j" "$edges"
	done
	echo ""
done

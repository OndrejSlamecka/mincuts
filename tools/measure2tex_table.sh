#!/usr/bin/env bash

if [[ $# -ne 1 ]] || [[ $1 == "-h" ]]; then
	echo "-t, -c -- input: a file produced by measure_on_graph.sh tool"
	echo "-t     -- table with running time"
	echo "-c     -- table with # of produced bonds"
	echo "-n     -- input: a file produced by measure_noncanonicity.sh tool"
	echo "       -- table with <(# of noncan.) / (# of can.) * 100> '%'"
	echo "Output: TeX table of the result"
	exit 1
fi

argv0=$1

# http://askubuntu.com/questions/179898/how-to-round-decimals-using-bc-in-bash
round()
{
	echo $(env printf %.$2f $(echo "scale=$2;(((10^$2)*$1)+0.5)/(10^$2)" | bc))
};

# Process the input
components=2
edges=1
declare -A table

while read line
do
	if [[ "$line" != "#"* && "$line" != "" ]]; then
		split_line=($line)
		c=${split_line[0]}
		e=${split_line[1]}

		# Process according to use case
		if [[ $argv0 == "-t" ]]; then
			table[$c,$e]=${split_line[2]}
		fi

		if [[ $argv0 == "-c" ]]; then
			table[$c,$e]=${split_line[3]}
		fi

		if [[ $argv0 == "-n" ]]; then
			r=$(echo "${split_line[2]} / ${split_line[3]} * 100 - 100" | bc -l)
			r=$(round "$r" 3)
			table[$c,$e]=$r
		fi

		# Save the biggest # of components and # of edges
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

#!/usr/bin/env bash

if [[ $# -ne 1 ]]; then
	echo "Usage: $0 <path to csv file>"
	exit 1
fi

if [[ ! -f "$1" ]]; then
	echo "File '$1' not found"
	exit 2
fi

NAME=$(basename "$1" | sed 's/\.csv$//')

echo "graph $NAME {"

while read LINE; do
	A=(${LINE//[;,]/ })
	echo -ne '\t'
	echo "${A[1]} -- ${A[2]} [label = \"${A[0]}\"];"
done < $1

echo "}"

#!/bin/bash

# todo: help

NAME=$(basename $1 | sed 's/\.csv$//')

echo "graph $NAME {"

while read LINE; do
	A=(${LINE//;/ })
	echo -ne '\t'
	echo "${A[1]} -- ${A[2]} [label = \"${A[0]}\"];"
done < $1

echo "}"

#!/bin/bash

if [[ $# -ne 3 ]]; then
	echo "Usage: $0 <edge_file.csv> <cut size bound> <# of components>"
	exit 1
fi

EDGES=$2
COMPONENTS=$3

mincuts $1 $EDGES $COMPONENTS      > tmp/tester_mincuts
mincuts $1 $EDGES $COMPONENTS -bfc > tmp/tester_bf

echo "mincuts / bf"
cutdiff tmp/tester_mincuts tmp/tester_bf
echo "bf / mincuts"
cutdiff tmp/tester_bf tmp/tester_mincuts


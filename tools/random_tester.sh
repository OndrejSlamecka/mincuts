#!/bin/bash

for i in {1..5000}
do	
	./tools/random_graph.sh 25 3 > "tmp/tester_in.csv"
	R=$(./tools/test_on_graph.sh tmp/tester_in.csv)
	L=$(echo "$R" | wc -l)
	if [[ L -eq 2 ]]; then
		echo -n "."
	else
		echo "$R";
		break;
	fi
done

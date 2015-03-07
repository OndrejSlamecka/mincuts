#!/bin/bash

LINES=$(wc -l $2 | awk '{print $1}')

for i in {1..5000}
do
	R=$[ 1 + $[ RANDOM % LINES ]]
	CUT=$(cat $2 | awk "FNR == $R")
	CMD="./build/mincuts $1 -imc $CUT"
	IS=$($CMD)
	if [ "$IS" != "true" ]; then
		echo $CMD
		echo $CUT
		echo $IS
		break
	else
		echo -ne "."
	fi
done

echo "DONE, nothing but dots above means OK"

#!/bin/bash

LINES=$(wc -l $2 | awk '{print $1}')

AWK_CMD=""
for i in {1..5000}
do
	R=$[ 1 + $[ RANDOM % LINES ]]
	AWK_CMD="FNR == $R || $AWK_CMD"
done

AWK_CMD=${AWK_CMD:0:-3}
CUTS=$(cat $2 | awk "$AWK_CMD")

for CUT in $CUTS
do
	CMD="./build/cutcheck $1 -imc $CUT"
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

echo ""
echo "DONE, nothing but dots above means OK"

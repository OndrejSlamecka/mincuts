
EDGES=$2
COMPONENTS=$3

mincuts $1 $EDGES $COMPONENTS      > tmp/tester_mincuts
mincuts $1 $EDGES $COMPONENTS -bfc > tmp/tester_bf

echo "mincuts / bf"
cutdiff tmp/tester_mincuts tmp/tester_bf
echo "bf / mincuts"
cutdiff tmp/tester_bf tmp/tester_mincuts


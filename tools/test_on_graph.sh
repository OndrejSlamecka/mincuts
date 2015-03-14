

mincuts $1 3 3      > tmp/tester_mincuts
mincuts $1 3 3 -bfc > tmp/tester_bf

echo "mincuts / bf"
cutdiff tmp/tester_mincuts tmp/tester_bf
echo "bf / mincuts"
cutdiff tmp/tester_bf tmp/tester_mincuts



Installation
------------

* Install OGDF Snapshot 2014-03-07 into `~/ogdf`
* Run `make`


TODO: Completness check against all combinations
-----------

Let %=% be an equiv. relation s.t. a %=% b <=> #components(G\a) = #components(G\b).
Let C = <cuts \subseteq combinations> and C_j be the class of %=% with index j.
Let K = cuts gen. by my mincuts implementation, K_j --||--.

for j in range(1,k):
	for each c in C_j:
		if some permutation* of some subset of c is contained: it's ok
		if not: report trouble

(* don't check all permutations, just order the result by edge index)


Notes to cannonical generation implementation
---------------------------------------------

Assumptions:
* Each cut X is ordered w.r.t. edge indicies, thus (12,15,17) (while (15,12) is not correct)



Tools
-----

**names2ids**

Converts from `ClosureSim` output format to `mincuts` format.

	./tools/names2ids.py data/zlin-silnice.csv < temp/closuresim/results-2.csv > ires-2

**size_split**

Splits `mincuts` results file by cut size.

**cutdiff**

Performs set difference on sets of cuts. If ~ is passed as a second parameter then it prints set out A\\{} (so if file A contains permutations of some cut only one representant of the cut will be printed).

	Usage: ./build/cutdiff <file A with cuts> [<file B with cuts> | ~]
	Output: Set difference A\B. Each cut of A and B is considered a set. '~' can be used to use empty set instead of second file
	Expected file format: lines of edge indicies separated by commas (e.g. 1,2,3)

-----------------------

http://www.fi.muni.cz/~xsvobo38/closuresim/


Installation
------------

* Install OGDF Snapshot 2014-03-07 into `~/.bin/ogdf` (or different directory but edit Makefile)
* Run `make`

Input/output file specification
-------------------------------

**Input** Each line contains one edge described in format `<edge id>;<source node id>;<target node id>`

**Output** Each line contains one cut: edge indicies joined by comma (e.g. `1,2,3`)

Run
---

**mincuts**

	Usage:	./build/mincuts <edge_file.csv> <cut size bound> <max components> [-bfc]

			-bfc --	use bruteforcing of all combinations instead of
					circuit-cocircuit algorithm


**cutcheck**

	Usage:	./build/cutcheck <edge_file.csv> [-imc, --ismincut <list>] [-cc <list>]
			[-rcc <cuts file>] [-tcc[f] <cuts file>]

			-imc    -- verifies if <list> is cut and minimal one
			-cc     -- computes # of components of G\<list>
			-rcc    -- randomized cuts check
			-tcc[f] -- total cuts check, 'f' counts all failures

	*list* is comma separated list of edge indicies.

**cutdiff**

	Usage: ./build/cutdiff <file A with cuts> [<file B with cuts> | ~]
	Output:
		* Set difference A\B (note that B\A is not computed)
		* Each cut of A and B is considered a set
		* '~' replaces second file with empty set

**names2ids**

Converts from [ClosureSim](http://www.fi.muni.cz/~xsvobo38/closuresim/) output format to `mincuts` format.

	./tools/names2ids.py data/zlin-silnice.csv < temp/closuresim/results-2.csv > ires-2

**size_split**

Splits `mincuts` results file by cut size.

------------

Work in progress: Notes to cannonical generation implementation
---------------------------------------------

Assumptions:
* Each cut X is ordered w.r.t. edge indicies, thus (12,15,17) (while (15,12) is not correct)


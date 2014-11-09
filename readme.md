
Installation
------------

* Install OGDF Snapshot 2014-03-07 into `~/ogdf`
* Run `make`

Tools
-----

**names2ids**

Converts from `ClosureSim` output format to `mincuts` format.

	./tools/names2ids.py data/zlin-silnice.csv < temp/closuresim/results-2.csv > ires-2

**size_split**

Splits `mincuts` results file by cut size.

**order & rm_permutations**

Work only with cuts of size 2.

	./tools/rm_permutations.py < temp/myop/zlin_2bySize/2 | ./tools/order.py 


-----------------------

http://www.fi.muni.cz/~xsvobo38/closuresim/


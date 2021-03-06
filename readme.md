This is a proof of concept implementation of CircuitCocircuit algorithm
for finding all small k-way bonds in a graph. The algorithm was devised
    by prof. Petr Hliněný.

A detailed description of the algorithm can be found in
[our paper](http://www.slamecka.cz/fi/bc_thesis/multicut.pdf).

Installation
------------

With GCC `>= 4.8.2`:

* Install OGDF into `~/.libs/ogdf` (or elsewhere but edit Makefile)
* Run `git clone git@github.com:OndrejSlamecka/mincuts.git && cd mincuts`
* Run `make`, the binaries will be in `bin/`

To install OGDF you can do:

    # install cmake, if you don't have it
    mkdir ~/.libs && cd ~/.libs
    git clone git@github.com:ogdf/ogdf.git && cd ogdf
    ccmake . # Disable OGDF_DEBUG unless you need it
    make


You may need to do
`git reset --HARD 6e4ba4b7f2110cbead8c426eee60a483fbe8feb0`
in the OGDF directory if you have downloaded newer but incompatible
version of OGDF.


Input/output file specification
-------------------------------

**Input** Each line contains one edge described in format
`<edge id>;<source node id>;<target node id>`

**Output** Each line contains one cut: edge indicies joined by comma
(e.g. `1,2,3`)

Run
---

Generate all 3-bonds with up to 4 edges of the road network of Zlin Region:


    ./bin/mincuts sample-data/zlin.csv 4 3



Overview of tools
-----------------

In `bin/`:

**mincuts**

    Usage:    mincuts <edge_file.csv> <cut size bound> <# of components> [-b]

        <# of components> can be exact or range (e.g. 2-3)
        -b --    use bruteforcing of all combinations instead of
                CircuitCocircuit algorithm


**cutcheck**


    Usage:    cutcheck <edge_file.csv> [-imc, --ismincut <list>] [-cc <list>]
        [-rcc <cuts file>] [-tcc[f] <cuts file>]

        -imc    -- verifies whether <list> is a cut and a minimal one
        -cc     -- computes the number of components of G\<list>
        -rcc    -- randomized cut checker
        -tcc[f] -- total cut checker, 'f' counts all failures

    *list* is a comma separated list of edge indicies.


**cutdiff**

    Usage: cutdiff <file A with cuts> [<file B with cuts> | ~]
    Output:
        * Set difference A\B (note that B\A is not computed)
        * Each cut of A and B is considered a set
        * '~' replaces second file with empty set

**tester**

    Usage:    tester <cut size bound> <# of components>
            [-r, --randomized <# of nodes> <min>-<max edges>] [-tN]

        This program tests CircuitCocircuit implementation.
        Expects graphs in nauty's graph6 format on stdin.

        First two arguments are used for any graph being tested.
        <# of components> can be exact or range (e.g. 2-3)
        -r generate random graphs (won't use stdin input)
        <max edges> is not strict (if the random graph is disconnected
            then we add edges to connect it)
        -tN use N threads (by default N == 2)

One can use this tester with program `geng` from package
[nauty](http://pallini.di.uniroma1.it/) to systematically test this
implementation. E.g. run `geng -cq 8 | tester 4 4` to test all graphs on
eight nodes. For testing bigger classes of graphs (like graphs on 10
vertices) it is recommended to use the `res/mod` option of `geng`.

**cutuniq**

    Usage:    cutuniq <file with cuts>

            Outputs to stdout the set of cuts without permutations.

Requires the `boost` C++ library. Build by `make cutuniq`. This tool is
memory efficient (at the cost of being IO expensive).

-----------

In `tools/`:

**csv2dot.sh**

Reads csv file with graph (path to file given in first argument) and
prints it in the dot format (used by GraphViz).

**test_on_graph.sh**

Compares CircuitCocircuit implementation output with cuts computed by
"brute-force" algorithm generating all combinations of edges which form
a minimal cut.

**geng_tester.sh**

Uses tool `geng` from package `nauty` to generate all the connected
graphs and passes them to `tester` (see above in `bin/`). It starts from
2 vertices and continues towards 32 vertices (which would take forever
to process).

**names2ids.py**

Converts from [ClosureSim](http://www.fi.muni.cz/~xsvobo38/closuresim/)
output format to `mincuts` format.

    ./tools/names2ids.py data/zlin-silnice.csv < temp/closuresim/results-2.csv > ires-2


**size_split.py**

Splits `mincuts` results file by cut size.

**graph6_to_csv.py**

Reads graph in `graph6` format (used by `nauty`) and outputs a CSV to
stdout. If filepath is passed as the only parameter then the output is
directed into this file.

This tool uses a bit of code from the [NetworkX
project](https://networkx.github.io/), which is distributed under BSD
license.

**Runtime measurement related tools**

* `measure_k_m_var.sh` -- runs `mincuts` with different values of `k`
  (in `k`-bonds) and `m` (max # of elements of the `k`-bonds)
* `measure2tex_k_m_var.sh` -- formats the output of the above tool into
  a TeX table
* `rtm2bonds_gen_acc_to_quartile.py` -- splits the records in the output
  of `mincuts-rtm` (see below "Measure runtime") by quartiles of the
  time axis, sums # of bonds generated by the records grouped by ranges
  marked by the quartiles and outputs a TeX table

Data
----

**Graph 5**

`Graph 5` is good to understand how the algorithm works. It can be found
in `sample-data/graph5.csv`.

![Graph 5](sample-data/graph5.png)

**Zlín Region**

Road network of the [Zlín
Region](http://en.wikipedia.org/wiki/Zl%C3%ADn_Region) can be found in
`sample-data/zlin.csv`

**Getting new data from OpenStreetMap**

You can export data from [OpenStreetMap](https://www.openstreetmap.org/)
and process them with
[osm4routing](https://github.com/Tristramg/osm4routing) to get `csv`
file.

You will probably want to select just data relevant to you. For example
extract just certain roads:

    $ awk -F',' 'NR > 1 {if ($5 > "1") { print $1";"$2";"$3 }}' edges_raw.csv

This excludes edges which have 0 or 1 in the fifth column and thus are
forbidden for cars or they are residential streets. See osm4routing page
for detailed information.

**Note** that if you want to analyze the road network of anything bigger
than few cities then the OSM data has the problem that each small road
junction has its own node which makes analysis of anything bigger
impossible. (If you solve the problem and extract a graph where each
node represents a single city then let me know!)

Measure runtime
---------------

One can measure the runtime of the algorithm implementation by building
the program with `make mincuts-rtm` (this requires the `boost` C++
library to be installed). All runs will then produce `mincuts_rtm.log`
file. Each line of that file contains a record about change of state
when some edge `e` is added to `X` in an `extendBond` call (note that if
your `k`-bonds have `k > 2` then `extendBond` calls are nested). Line
contents (tab separated):

* current stage of the algorithm (note `j`-bonds are created in stage `j-1`)
* time spent in the `genStage` call in milliseconds
* number of bonds generated with given `X` union `{e}`

Measure lengths of paths
------------------------

Compile with `make mincuts-pathslengths`. `mincuts` will output stage
number, level of `genStage` in the stage and length of the path between
`V(T_r)` and `V_b` found by `shortestPath` on each line, all separated
by spaces.

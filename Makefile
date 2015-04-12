ogdf_path = ~/.bin/ogdf

CXXFLAGS=-std=c++11 -pedantic -Wall -Wextra -O3 $(EXTRA_CXXFLAGS)
LINKS=-I $(ogdf_path)/include -L $(ogdf_path)/_debug -lOGDF -lpthread

all: mincuts cutdiff cutcheck tester

mincuts: src/helpers.cpp src/circuitcocircuit.cpp src/mincuts.cpp
	g++ -o bin/$@ $(CXXFLAGS) $^ $(LINKS)

cutdiff: src/cutdiff.cpp
	g++ -o bin/$@ $(CXXFLAGS) $^

cutcheck: src/helpers.cpp src/cutcheck.cpp
	g++ -o bin/$@ $(CXXFLAGS) $^ $(LINKS)

tester: src/circuitcocircuit.cpp src/helpers.cpp src/tester.cpp
	g++ -o bin/$@ $(CXXFLAGS) $^ $(LINKS)


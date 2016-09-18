ogdf_path = ~/.libs/ogdf

CXXFLAGS=-std=c++11 -pedantic -Wall -Wextra -O3 $(EXTRA_CXXFLAGS)
LINKS=-I $(ogdf_path)/include -L $(ogdf_path) -lOGDF -lpthread
DYNCON_FILES=src/dyncon/rnb_tree.c src/dyncon/rnbw_tree.c src/dyncon/ed_tree.c src/dyncon/et_tree.c src/dyncon/dyn_con.c

FAKEVAR:=$(shell mkdir -p bin)

.PHONY: all

all: mincuts cutdiff cutcheck tester

mincuts-rtm: CXXFLAGS += -DMEASURE_RUNTIME
mincuts-rtm: LINKS    += -lboost_chrono -lboost_system
mincuts-rtm: mincuts

mincuts-pathslengths: CXXFLAGS += -DMEASURE_PATHS_LENGTHS
mincuts-pathslengths: mincuts

mincuts: src/helpers.cpp src/circuitcocircuit.cpp $(DYNCON_FILES) src/mincuts.cpp
	$(CXX) -o bin/$@ $(CXXFLAGS) $^ $(LINKS)

cutdiff: src/cutdiff.cpp
	$(CXX) -o bin/$@ $(CXXFLAGS) $^

cutcheck: src/helpers.cpp src/cutcheck.cpp
	$(CXX) -o bin/$@ $(CXXFLAGS) $^ $(LINKS)

tester: src/helpers.cpp src/circuitcocircuit.cpp $(DYNCON_FILES) src/tester.cpp
	$(CXX) -o bin/$@ $(CXXFLAGS) $^ $(LINKS)

cutuniq: src/cutuniq.cpp
	$(CXX) -o bin/$@ $(CXXFLAGS) $^ -lboost_filesystem -lboost_system


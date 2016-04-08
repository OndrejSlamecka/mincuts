ogdf_path = ~/.libs/ogdf

CXX=g++
CXXFLAGS=-std=c++11 -pedantic -Wall -Wextra -O3 $(EXTRA_CXXFLAGS)
LINKS=-I $(ogdf_path)/include -L $(ogdf_path) -lOGDF -lpthread

FAKEVAR:=$(shell mkdir -p bin)

.PHONY: all

all: mincuts cutdiff cutcheck tester

mincuts-rtm: CXXFLAGS += -DMEASURE_RUNTIME
mincuts-rtm: LINKS    += -lboost_chrono -lboost_system
mincuts-rtm: mincuts

mincuts-pathslengths: CXXFLAGS += -DMEASURE_PATHS_LENGTHS
mincuts-pathslengths: mincuts

mincuts: src/helpers.cpp src/circuitcocircuit.cpp src/mincuts.cpp
	$(CXX) -o bin/$@ $(CXXFLAGS) $^ $(LINKS)

cutdiff: src/cutdiff.cpp
	$(CXX) -o bin/$@ $(CXXFLAGS) $^

cutcheck: src/helpers.cpp src/cutcheck.cpp
	$(CXX) -o bin/$@ $(CXXFLAGS) $^ $(LINKS)

tester: src/circuitcocircuit.cpp src/helpers.cpp src/tester.cpp
	$(CXX) -o bin/$@ $(CXXFLAGS) $^ $(LINKS)

cutuniq: src/cutuniq.cpp
	$(CXX) -o bin/$@ $(CXXFLAGS) $^ -lboost_filesystem -lboost_system


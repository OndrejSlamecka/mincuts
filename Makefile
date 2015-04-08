ogdf_path = ~/.bin/ogdf
nauty_path = ~/.bin/nauty25r9

CXXFLAGS=-std=c++11 -pedantic -Wall -Wextra -O3 $(EXTRA_CXXFLAGS) 
LINKS=-I $(ogdf_path)/include -L $(ogdf_path)/_debug -lOGDF -lpthread

ifeq "$(wildcard $(nauty_path) )" ""
	NAUTY=
	CXXFLAGS_TESTER=CXXFLAGS
else
	NAUTY=-DNAUTY -DOUTPROC=receiveGraph -DGENG_MAIN=geng_main -I $(nauty_path) -L $(nauty_path)
	NAUTY_FILES=$(nauty_path)/geng.c $(nauty_path)/gtools.o $(nauty_path)/nauty1.o $(nauty_path)/nautil1.o $(nauty_path)/naugraph1.o $(nauty_path)/schreier.o $(nauty_path)/naurng.o
	CXXFLAGS_TESTER=-std=c++11 -Wno-write-strings -O3 $(EXTRA_CXXFLAGS)
endif

all: mincuts cutdiff cutcheck tester

mincuts: src/helpers.cpp src/circuitcocircuit.cpp src/mincuts.cpp
	g++ -o bin/$@ $(CXXFLAGS) $^ $(LINKS)

cutdiff: src/cutdiff.cpp
	g++ -o bin/$@ $(CXXFLAGS) $^

cutcheck: src/helpers.cpp src/cutcheck.cpp
	g++ -o bin/$@ $(CXXFLAGS) $^ $(LINKS)

tester: src/circuitcocircuit.cpp src/helpers.cpp src/tester.cpp
	g++ -o bin/$@ $(CXXFLAGS_TESTER) $(NAUTY) $(NAUTY_FILES) $^ $(LINKS)

ifeq "$(NAUTY)" ""
	@echo "-- nauty not found in $(nauty_path), tester option -c disabled"
else
	@echo "-- tester compiled with nauty, option -c enabled"
endif

ogdfPath = ~/.bin/ogdf

flags = -std=c++11 -pedantic -Wall -Wextra
links = -lOGDF -lpthread
ogdf = -L $(ogdfPath)/_debug -I $(ogdfPath)/include

all: mincuts cutdiff cutcheck tester

mincuts: src/helpers.cpp src/graphcoloring.cpp src/circuitcocircuit.cpp src/mincuts.cpp
	g++ $(flags) $(ogdf) src/helpers.cpp src/graphcoloring.cpp src/circuitcocircuit.cpp src/mincuts.cpp -o bin/mincuts $(links)

cutdiff: src/cutdiff.cpp
	g++ $(flags) src/cutdiff.cpp -o bin/cutdiff 

cutcheck: src/helpers.cpp src/cutcheck.cpp
	g++ $(flags) $(ogdf) src/helpers.cpp src/cutcheck.cpp -o bin/cutcheck $(links)

tester: src/graphcoloring.cpp src/circuitcocircuit.cpp src/helpers.cpp src/tester.cpp
	g++ $(flags) $(ogdf) src/graphcoloring.cpp src/circuitcocircuit.cpp src/helpers.cpp src/tester.cpp -o bin/tester $(links)

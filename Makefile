ogdfPath = ~/.bin/ogdf

flags = -std=c++11 -pedantic -Wall -Wextra
links = -lOGDF -lpthread
ogdf = -L $(ogdfPath)/_debug -I $(ogdfPath)/include

all: main cutdiff cutcheck

main: src/helpers.cpp src/graphcoloring.cpp src/circuitcocircuit.cpp src/main.cpp
	g++ $(flags) $(ogdf) src/helpers.cpp src/graphcoloring.cpp src/circuitcocircuit.cpp src/main.cpp -o bin/mincuts $(links)

cutdiff: src/cutdiff.cpp 
	g++ $(flags) src/cutdiff.cpp -o bin/cutdiff

cutcheck: src/helpers.cpp src/cutcheck.cpp 
	g++ $(flags) $(ogdf) src/helpers.cpp src/cutcheck.cpp -o bin/cutcheck $(links)

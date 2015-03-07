ogdfPath = ~/.bin/ogdf

flags = -std=c++11 -pedantic -Wall -Wextra
links = -lOGDF -lpthread
ogdf = -L $(ogdfPath)/_debug -I $(ogdfPath)/include

all: main bruteforce cutdiff cutcheck

main: src/graphcoloring.cpp src/main.cpp
	g++ $(flags) $(ogdf) src/graphcoloring.cpp src/main.cpp -o build/mincuts $(links)

bruteforce: src/bruteforce.cpp
	g++ $(flags) $(ogdf) src/bruteforce.cpp -o build/bruteforce $(links)

cutdiff: src/cutdiff.cpp
	g++ $(flags) src/cutdiff.cpp -o build/cutdiff

cutcheck: src/cutcheck.cpp
	g++ $(flags) $(ogdf) src/cutcheck.cpp -o build/cutcheck $(links)

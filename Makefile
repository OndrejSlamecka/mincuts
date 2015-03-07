ogdfPath = ~/.bin/ogdf

flags = -std=c++11 -pedantic -Wall -Wextra
links = -lOGDF -lpthread
ogdf = -L $(ogdfPath)/_debug -I $(ogdfPath)/include

all: main bruteforce cutdiff

main: src/graphcoloring.cpp src/main.cpp
	g++ $(flags) $(ogdf) src/graphcoloring.cpp src/main.cpp -o build/mincuts $(links)

bruteforce: src/bruteforce.cpp
	g++ $(flags) $(ogdf) src/bruteforce.cpp -o build/bruteforce $(links)

cutdiff: tools/cutdiff.cpp
	g++ $(flags) tools/cutdiff.cpp -o build/cutdiff

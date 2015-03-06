ogdfPath = ~/.bin/ogdf

flags = -std=c++11 -pedantic -Wall -Wextra
links = -lOGDF -lpthread
ogdf = -L $(ogdfPath)/_debug -I $(ogdfPath)/include

all: main bruteforce

main: src/graphcoloring.cpp src/main.cpp
	g++ $(flags) $(ogdf) src/graphcoloring.cpp src/main.cpp -o build/mincuts $(links)

bruteforce: src/bruteforce.cpp
	g++ $(flags) $(ogdf) src/bruteforce.cpp -o build/bruteforce $(links)

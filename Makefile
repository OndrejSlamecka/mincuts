ogdfPath = ~/libs/ogdf

flags = -std=c++11 -pedantic -Wall -Wextra
links = -lOGDF -lpthread
ogdf = -L $(ogdfPath)/_debug -I $(ogdfPath)/include

all:
	g++ $(flags) $(ogdf) src/graphcoloring.cpp src/main.cpp -o build/mincuts $(links)
	

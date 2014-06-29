ogdfPath = ~/ogdf

flags = -std=c++11 -pedantic -Wall -Wextra
links = -lOGDF -lpthread
ogdf = -L $(ogdfPath)/_debug -I $(ogdfPath)/include

all:
	g++ $(flags) $(ogdf) main.cpp -o mincuts $(links)
	

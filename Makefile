MAIN = ./src/main.cpp

all:
	g++ $(MAIN) -o main -std=c++11 -O3

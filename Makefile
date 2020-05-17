all: mcts

mcts: mcts.cpp
	g++ mcts.cpp -o mcts -O2 -std=gnu++17 -Wall

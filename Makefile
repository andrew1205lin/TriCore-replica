FLAGS = -O3 -g -std=c++17

all: graph


graph: graph.o mmio.o
	g++ graph.o mmio.o ${FLAGS} -Wall -Werror -o graph

graph.o: graph.cpp
	g++ -c graph.cpp ${FLAGS} -o graph.o

mmio.o: mmio.c
	g++ -c mmio.c ${FLAGS} -o mmio.o

clean:
	rm -rf *.o graph

# definindo variavel para o compilador usado
CP=g++

graph: graph.o
	@$(CP) graph.o -o graph

graph.o: graph.cpp
	@$(CP) -c graph.cpp

all:
	@$(CP) -o graph graph.cpp

run:
	@./graph

clean:
	@rm -rf graph *.o

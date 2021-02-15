all: bst test

bst: bst.c
	g++ -o bst bst.c -pthread -g -O0

test: UnitTest.c
	g++ -o test UnitTest.c -pthread
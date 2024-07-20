all:
	clang++ -O2 -Wall -g -march=native -o satt STiltedDriverV4.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp
debug:
	g++ -O2 -g -Wall -march=native -o stest STiltedDriverV4.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp
all:
	clang++ -O2 -Wall -g -march=native -o stilted-26R STiltedDriverV4.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp
debug:
	clang++ -g -Wall -march=native -o stest STiltedDriverV4.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp
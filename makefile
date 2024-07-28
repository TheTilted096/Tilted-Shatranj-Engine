all:
	clang++ -O2 -Wall -static -g -march=native -o stilted-27-release STiltedDriverV3.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp
debug:
	g++ -O2 -g -Wall -march=native -o stest STiltedDriverV3.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp
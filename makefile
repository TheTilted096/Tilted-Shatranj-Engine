all:
	clang++ -O2 -Wall -static -g -march=native -o stilted-27-mob STiltedDriverV3.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp

debug:
	g++ -O2 -g -Wall -march=native -o stest STiltedDriverV3.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp

autoplay:
	g++ -O2 -g -Wall -march=native -o sautoplay STiltedAutoPlay.cpp STiltedAutoLibV2.cpp STiltedSearchEvalV4.cpp STiltedMoveGenV5.cpp

multi:
	g++ -g -Wall -march=native -o sspsadebug STiltedSPSAV3.cpp STiltedAutoLibV2.cpp STiltedSearchEvalV4.cpp STiltedMoveGenV5.cpp

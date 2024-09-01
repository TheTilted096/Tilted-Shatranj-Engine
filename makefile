all:
	clang++ -O2 -Wall -static -g -march=native -o stilted-27-mob STiltedDriverV3.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp

debug:
	clang++ -O2 -Wall -static -g -march=native -o stilted-27-mob3 STiltedDriverV3.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp

spsa:
	g++ -O2 -g -Wall -march=native -o sspsa STiltedSPSAV3.cpp STiltedAutoLibV2.cpp STiltedSearchEvalV4.cpp STiltedMoveGenV5.cpp

autoplay:
	g++ -O2 -g -Wall -march=native -o sautoplay STiltedAutoPlayV2.cpp STiltedAutoLibV2.cpp STiltedSearchEvalV4.cpp STiltedMoveGenV5.cpp

tuna:
	g++ -O2 -g -Wall -march=native -o stuna STiltedTunaV2.cpp STiltedSearchEvalV4.cpp STiltedMoveGenV5.cpp





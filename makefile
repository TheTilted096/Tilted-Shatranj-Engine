# open bench is goated

EXE := Tilted28
#EVALFILE

SOURCES := STiltedDriverV3.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp

CXX :=  clang++

CXXFLAGS := -O2 -Wall -static -g -march=native -pthread

SUFFIX := .exe

OUT := $(EXE)$(SUFFIX)

$(EXE): $(SOURCES)
	$(CXX) $^ $(CXXFLAGS) -o $(OUT)

#release:
#	clang++ -O2 -Wall -static -g -march=native -o stilted-28-release STiltedDriverV3.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp

debug:
	clang++ -O2 -Wall -static -g -march=native -o stilted-28-dev STiltedDriverV3.cpp STiltedMoveGenV5.cpp STiltedSearchEvalV4.cpp

spsa:
	g++ -O2 -g -Wall -march=native -o sspsa STiltedSPSAV3.cpp STiltedAutoLibV2.cpp STiltedSearchEvalV4.cpp STiltedMoveGenV5.cpp

autoplay:
	g++ -O2 -g -Wall -march=native -o sautoplay STiltedAutoPlayV2.cpp STiltedAutoLibV2.cpp STiltedSearchEvalV4.cpp STiltedMoveGenV5.cpp

tuna:
	g++ -O2 -g -Wall -march=native -o stuna STiltedTunaV2.cpp STiltedSearchEvalV4.cpp STiltedMoveGenV5.cpp



all: movegen.o
	g++ -march=native -g -o stest STiltedDriverV4.cpp movegen.o

movegen.o:
	g++ -march=native -g -c STiltedMoveGenV5.cpp -o movegen.o

clean:
	del stest.exe *.o

debug:
	make clean
	make

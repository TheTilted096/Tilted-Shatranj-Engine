all:
	g++ -O2 -o stilteddev "STiltedDriver.cpp"
	./stilteddev

release:
	g++ -O2 -static -o stilted-2-release "STiltedDriver.cpp"

devbuild:
	g++ -O2 -o stilteddev "STiltedDriver.cpp"

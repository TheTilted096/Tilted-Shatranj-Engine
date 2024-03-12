all:
	g++ -O2 -o stilteddev "STiltedDriver.cpp"
	./stilteddev

release:
	g++ -O2 -static -o stilted-release-u311 "STiltedDriver.cpp"

devbuild:
	g++ -O2 -o stilteddev "STiltedDriver.cpp"

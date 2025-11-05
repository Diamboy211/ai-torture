CXX ?= g++
CXXFLAGS ?= -g -I../build_liboai/include
LDFLAGS ?= -L../build_liboai/lib

jail: main.o
	${CXX} ${LDFLAGS} -o jail main.o -loai -lcurl
main.o: main.cc
	${CXX} ${CXXFLAGS} -c -o main.o main.cc

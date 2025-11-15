CXX ?= g++
CXXFLAGS ?= -g -I../build_liboai/include
LDFLAGS ?= -L../build_liboai/lib

.PHONY: all run clean

all: jail

run: jail
	./jail

-include rfb_handler.d main.d ocr.d vgafont16.d helper.d

jail: main.o helper.o rfb_handler.o vgafont16.o ocr.o
	${CXX} ${LDFLAGS} -o jail main.o helper.o rfb_handler.o vgafont16.o ocr.o -loai -lcurl -lvncclient -llept
rfb_handler.o: rfb_handler.cc
	${CXX} ${CXXFLAGS} -MMD -c -o rfb_handler.o rfb_handler.cc
main.o: main.cc
	${CXX} ${CXXFLAGS} -MMD -c -o main.o main.cc
ocr.o: ocr.cc
	${CXX} ${CXXFLAGS} -MMD -c -o ocr.o ocr.cc
vgafont16.o: vgafont16.cc
	${CXX} ${CXXFLAGS} -MMD -c -o vgafont16.o vgafont16.cc
helper.o: helper.cc
	${CXX} ${CXXFLAGS} -MMD -c -o helper.o helper.cc

clean:
	rm -f *.o *.d jail

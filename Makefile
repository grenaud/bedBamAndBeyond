
CXX      = g++   
LIBGAB   = /home/gabriel_renaud/lib/

CXXFLAGS = -Wall -lm -O3 -lz -I${LIBGAB} -I${LIBGAB}/VCFparser/gzstream/ -I${LIBGAB}/VCFparser/gzstream/ -I/home/gabriel_renaud/Software/libharu/build/include/ -I/home/gabriel_renaud/Software/libharu/include/ -c
LDFLAGS  = -lz


all: bedBamAndBeyond 

bedBamAndBeyond.o:	bedBamAndBeyond.cpp
	${CXX} ${CXXFLAGS} bedBamAndBeyond.cpp


bedBamAndBeyond:	bedBamAndBeyond.o ${LIBGAB}utils.o  ${LIBGAB}VCFparser/gzstream/libgzstream.a /home/gabriel_renaud/Software/libharu/build/src/libhpdf.so
	${CXX} -o $@ $^ $(LDLIBS) $(LDFLAGS)

clean :
	rm -f bedBamAndBeyond.o bedBamAndBeyond



CXX      = g++ 
LIBGAB   = /home/gabriel_renaud/lib/
BAMTOOLS= /mnt/solexa/bin/bamtools-2.2.2/

CXXFLAGS = -Wall -lm -O3 -lz -I${LIBGAB} -I${BAMTOOLS}/include/ -I${LIBGAB}/VCFparser/gzstream/ -I${LIBGAB}/VCFparser/gzstream/ -I/home/gabriel_renaud/Software/libharu/build/include/ -I/home/gabriel_renaud/Software/libharu/include/ -c
LDFLAGS  = ${BAMTOOLS}/lib/libbamtools.a -lz


all: bedBamAndBeyond 

bedBamAndBeyond.o:	bedBamAndBeyond.cpp
	${CXX} ${CXXFLAGS} bedBamAndBeyond.cpp


bedBamAndBeyond:	bedBamAndBeyond.o ${LIBGAB}utils.o  ${LIBGAB}VCFparser/gzstream/libgzstream.a /home/gabriel_renaud/Software/libharu/build/src/libhpdf.so
	${CXX} -o $@ $^ $(LDLIBS) $(LDFLAGS)

clean :
	rm -f bedBamAndBeyond.o bedBamAndBeyond


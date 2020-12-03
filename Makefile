
CXX      = g++  #-g -pg 
#LIBGAB   = /home/gabriel_renaud/lib/
#BAMTOOLS= /mnt/solexa/bin/bamtools-2.2.2/
#BAMTOOLS= bamtools/)
#LIBGAB= $(realpath libgab/)
#LIBHARU= $(realpath libharu/)

CXXFLAGS = -Wall -lm -O3 -lz -Ilibgab/ -Ibamtools//include/ -Ibamtools//src/   -Ilibgab//gzstream/ -I libharu/include/  -c
#LDFLAGS  = bamtools//lib/libbamtools.a -lz
LDFLAGS  =    -lm  -lz

all: bedBamAndBeyond 


libgab/libgab.h:
	rm -rf libgab/
	git clone --recursive https://github.com/grenaud/libgab.git

libgab/libgab.a:  libgab/libgab.h
	cd libgab/ &&  make libgab.a && make -C gzstream/ && cd ../

bamtools/src/api/BamAlignment.h:
	rm -rf bamtools/
	git clone --recursive https://github.com/pezmaster31/bamtools.git

bamtools/build/src/api/libbamtools.a: bamtools/src/api/BamAlignment.h
	cd bamtools/ && mkdir -p build/  && cd build/ && cmake .. && make && cd ../..

bamtools/build/src/utils/CMakeFiles/BamTools-utils.dir/*cpp.o: bamtools/build/src/api/libbamtools.a

libharu/include/hpdf.h:
	rm -rf libharu/
	mkdir libharu/
	git clone --depth 1 https://github.com/libharu/libharu.git libharu/

libharu/src/.libs/libhpdf.a: libharu/include/hpdf.h
	cd libharu/ && ./buildconf.sh && ./configure && make

bedBamAndBeyond.o:	bedBamAndBeyond.cpp libharu/src/.libs/libhpdf.a libgab/libgab.a  libgab/gzstream/libgzstream.a bamtools/build/src/api/libbamtools.a
	${CXX} ${CXXFLAGS} bedBamAndBeyond.cpp


bedBamAndBeyond:	bedBamAndBeyond.o libgab/libgab.a  libharu/src/.libs/libhpdf.a libgab/libgab.a libgab/gzstream/libgzstream.a bamtools/build/src/api/libbamtools.a
	${CXX} -o $@ $^ $(LDLIBS) $(LDFLAGS)

clean :
	rm -f bedBamAndBeyond.o bedBamAndBeyond


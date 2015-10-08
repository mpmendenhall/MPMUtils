# assure correct shell is used
SHELL = /bin/sh

# apply implicit rules only for listed file types
.SUFFIXES:
.SUFFIXES: .c .cc .cpp .o

# base path to usual unixy folders (include, lib, etc),
# probably just '/usr/'... unless you are using Fink on Mac OS X and have your stuff in '/sw/'
OS_DIR = /usr/

# compiler command to use
CXXFLAGS = -std=c++11 -O3 -fPIC -pedantic -Wall -Wextra -I. -I../GeneralUtils/ -I/usr/include/lapacke/

WITH_LAPACKE = 1
ifdef WITH_LAPACKE
	# BLAS and LAPACK(E) libraries for matrix manipulation; also, need gfortran for LAPACK linking
	LDFLAGS += -llapacke -llapack -lblas -lgfortran
	CXXFLAGS += -DWITH_LAPACKE
endif

#
# things to build
#

objects = BlockCMat.o Clustering.o CMatrix.o ConvolverFactory.o \
	LAPACKE_Matrix.o LinMin.o Permutation.o TLS_Solver.o

all: libMPMMatrix.a

libMPMMatrix.a: $(objects)
	ar rs libMPMMatrix.a $(objects)
libMPMMatrix.so: $(objects)
	$(CXX) -shared $(CXXFLAGS) -o libMPMMatrix.so $(objects) $(LDFLAGS)

#
# cleanup
#

.PHONY: clean
clean:
	-rm -f *.o *.a *so

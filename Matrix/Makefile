# assure correct shell is used
SHELL = /bin/sh

# apply implicit rules only for listed file types
.SUFFIXES:
.SUFFIXES: .c .cc .cpp .o

# base path to usual unixy folders (include, lib, etc),
# probably just '/usr/'... unless you are using Fink on Mac OS X and have your stuff in '/sw/'
OS_DIR = /usr/

# compiler command to use
CXXFLAGS = -std=c++11 -O3 -fPIC -pedantic -Wall -Wextra -Wpedantic -I. -I../GeneralUtils/

WITH_LAPACKE = 1
ifdef WITH_LAPACKE
	# BLAS and LAPACK(E) libraries for matrix manipulation; also, need gfortran for LAPACK linking
	LDFLAGS += -llapacke -llapack -lblas -lgfortran
	CXXFLAGS += -DWITH_LAPACKE
endif

#
# things to build
#

objects = BlockCMat.o CMatrix.o LAPACKE_Matrix.o LinMin.o Permutation.o

all: libMPMMatrix.a

libMPMMatrix.a: $(objects)
	ar rs libMPMMatrix.a $(objects)

../GeneralUtils/libMPMGeneralUtils.a:
	+cd ../GeneralUtils; make

#
# cleanup
#

.PHONY: clean
clean:
	-rm -f libMPMMatrix.a
	-rm -f *.o
	-cd ../GeneralUtils; make clean

# assure correct shell is used
SHELL = /bin/sh

# apply implicit rules only for listed file types
.SUFFIXES:
.SUFFIXES: .c .cc .cpp .o
	 
# compiler command to use
CXXFLAGS = -std=c++11 -O3 -fPIC `root-config --cflags` -pedantic -Wall -Wextra -I. -I../GeneralUtils/
LDFLAGS =  -L../GeneralUtils/ -lMPMGeneralUtils `root-config --libs`

#
# things to build
#

objects = EnumerationFitter.o GraphicsUtils.o GraphUtils.o HistogramSequenceFitter.o MultiGaus.o OutputManager.o\
	PointCloudHistogram.o TChainScanner.o TSQLHelper.o SegmentSaver.o

all: libMPMROOTUtils.a

libMPMROOTUtils.a: ../GeneralUtils/libMPMGeneralUtils.a $(objects)
	ar rs  libMPMROOTUtils.a $(objects)

../GeneralUtils/libMPMGeneralUtils.a:
	+cd ../GeneralUtils; make

#
# cleanup
#

.PHONY: clean
clean:
	-rm -f libMPMROOTUtils.a
	-rm -f *.o
	-cd ../GeneralUtils; make clean

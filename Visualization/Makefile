# assure correct shell is used
SHELL = /bin/sh

# apply implicit rules only for listed file types
.SUFFIXES:
.SUFFIXES: .c .cc .cpp .o
	 
# compiler command to use
CXXFLAGS = -std=c++11 -O3 -fPIC `root-config --cflags` -pedantic -Wall -Wextra -I. -I../GeneralUtils/ -I../Matrix/
LDFLAGS =  -L../Matrix/ -lMPMMatrix -L../GeneralUtils/ -lMPMGeneralUtils -lGL -lglut

#
# things to build
#

objects = Visr.o

all: libMPMVis.a

libMPMVis.a: ../GeneralUtils/libMPMGeneralUtils.a ../Matrix/libMPMMatrix.a $(objects)
	ar rs  libMPMVis.a $(objects)

../GeneralUtils/libMPMGeneralUtils.a:
	+cd ../GeneralUtils; make

../Matrix/libMPMMatrix.a:
	+cd ../Matrix; make

#
# cleanup
#

.PHONY: clean
clean:
	-rm -f libMPMVis.a
	-rm -f *.o
	-cd ../GeneralUtils; make clean

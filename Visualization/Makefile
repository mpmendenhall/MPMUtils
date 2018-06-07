# assure correct shell is used
SHELL = /bin/sh

# apply implicit rules only for listed file types
.SUFFIXES:
.SUFFIXES: .c .cc .cpp .o

# compiler command to use
CXXFLAGS = -std=c++11 -O3 -fPIC `root-config --cflags` -pedantic -Wall -Wextra -I. -I../GeneralUtils/ -DWITH_OPENGL=1
LDFLAGS =  -lGL -lglut

#
# things to build
#

objects = Visr.o

all: libMPMVis.a testVisr

libMPMVis.a: $(objects)
	ar rs  libMPMVis.a $(objects)

# generic rule for everything else .cc
%: %.cc $(objects)
	$(CXX) $(CXXFLAGS) $< $(objects) $(LDFLAGS) -o $@

#
# cleanup
#

.PHONY: clean
clean:
	-rm -f libMPMVis.a testVisr
	-rm -f *.o

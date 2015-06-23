
# assure correct shell is used
SHELL = /bin/sh

# apply implicit rules only for listed file types
.SUFFIXES:
.SUFFIXES: .c .cc .cpp .o

# compiler command to use
CXXFLAGS = -std=c++11 -O3 -fPIC -pedantic -Wall -Wextra -I. 

#
# things to build
#

objects = ColorSpec.o ControlMenu.o DynamicHistogram.o FloatErr.o PathUtils.o ProgressBar.o QFile.o \
	RollingWindow.o SMExcept.o StringManip.o Stringmap.o TagCounter.o

all: libMPMGeneralUtils.a

libMPMGeneralUtils.a: $(objects)
	ar rs libMPMGeneralUtils.a $(objects)

#
# cleanup
#

.PHONY: clean
clean:
	-rm -f libMPMGeneralUtils.a
	-rm -f *.o

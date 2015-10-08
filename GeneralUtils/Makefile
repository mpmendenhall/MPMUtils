
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

objects = ColorSpec.o CompileVersion.o ControlMenu.o DynamicHistogram.o FloatErr.o PathUtils.o \
	ProgressBar.o SMFile.o RollingWindow.o SMExcept.o StringManip.o Stringmap.o \
	SVGGradientAxis.o TagCounter.o XMLBuilder.o

all: libMPMGeneralUtils.a libMPMGeneralUtils.so

libMPMGeneralUtils.a: $(objects)
	ar rs libMPMGeneralUtils.a $(objects)
libMPMGeneralUtils.so: $(objects)
	$(CXX) -shared -fPIC -o libMPMGeneralUtils.so $(objects)

.PHONY: CompileVersion.o
CompileVersion.o: 
	$(CXX) -c $(CXXFLAGS) -DGIT_SHA=$(shell git rev-parse -q HEAD) CompileVersion.cc -o CompileVersion.o

#
# cleanup
#

.PHONY: clean
clean:
	-rm -f *.o *.a *.so

# assure correct shell is used
SHELL = /bin/sh

# apply implicit rules only for listed file types
.SUFFIXES:
.SUFFIXES: .c .cc .cpp .o
	 
# compiler command to use
CXXFLAGS = -std=c++11 -O3 -fPIC `root-config --cflags` -pedantic -Wall -Wextra -I. -I../GeneralUtils/

objects = EnumerationFitter.o GraphicsUtils.o GraphUtils.o HistogramSequenceFitter.o \
	LinHistCombo.o MultiGaus.o OutputManager.o PluginSaver.o PointCloudHistogram.o \
	TChainScanner.o TObjCollector.o TSQLHelper.o SegmentSaver.o SplineFit.o

all: libMPMROOTUtils.a libMPMROOTUtils.so

libMPMROOTUtils.a: $(objects)
	ar rs  libMPMROOTUtils.a $(objects)
libMPMROOTUtils.so: $(objects)
	$(CXX) -shared -fPIC -o libMPMROOTUtils.so $(objects)

.PHONY: clean
clean:
	-rm -f *.o *.a *.so

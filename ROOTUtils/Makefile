# assure correct shell is used
SHELL = /bin/sh

# apply implicit rules only for listed file types
.SUFFIXES:
.SUFFIXES: .c .cc .cpp .cxx .o

# compiler command to use
CXXFLAGS = -std=c++11 -O3 -fPIC `root-config --cflags` -pedantic -Wall -Wextra -I. -I../GeneralUtils/

objects = DynamicPluginSaver.o EnumerationFitter.o GraphicsUtils.o GraphUtils.o HistogramSequenceFitter.o IterRangeFitter.o \
	LinHistCombo.o MultiGaus.o OutputManager.o PluginSaver.o PointCloudHistogram.o \
	SegmentSaver.o SplineFit.o TChainScanner.o TF1_Quantiles.o TObjCollector.o TSQLHelper.o mpmrootdict.o
# ROOT TObject-derived linkdef'd objects and header files
ldefobjs = TDynamicHistogram.o TRateCategories.o TRatechart.o TStripchart.o
ldefincs = CumulativeData.hh TCumulative.hh TCumulativeMap.hh TDynamicHistogram.hh TDynamicLogHistogram.hh \
	TRateCategories.hh TRatechart.hh TStripchart.hh LinkDef.h

all: libMPMROOTUtils.a libMPMROOTUtils.so

libMPMROOTUtils.a: $(objects) $(ldefobjs)
	ar rs  libMPMROOTUtils.a $(objects) $(ldefobjs)
libMPMROOTUtils.so: $(objects)
	$(CXX) -shared -fPIC -o libMPMROOTUtils.so $(objects) $(ldefobjs) -L ../GeneralUtils/ -lMPMGeneralUtils `root-config --glibs` -lMathMore -lconfig
mpmrootdict.o: LinkDef.h $(ldefobjs) $(ldefincs)
	rootcling -f mpmrootdict.cxx -c $(ldefincs)
	$(CXX) $(CXXFLAGS) -c mpmrootdict.cxx -o mpmrootdict.o

.PHONY: clean
clean:
	-rm -f *.o *.a *.so
	-rm -f mpmrootdict.cxx

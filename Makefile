# assure correct shell is used
SHELL = /bin/sh

# apply implicit rules only for listed file types
.SUFFIXES:
.SUFFIXES: .c .cc .cpp .o

# compiler command to use
CXXFLAGS = -std=c++11 -O3 -fPIC -pedantic -Wall -Wextra -Wpedantic -I. \
	-IGeneralUtils/ -IMatrix/ -IROOTUtils/

# things to build

all: GeneralUtils/libMPMGeneralUtils.a ROOTUtils/libMPMROOTUtils.a Matrix/libMPMMatrix.a

GeneralUtils/libMPMGeneralUtils.a:
	+cd GeneralUtils; make

ROOTUtils/libMPMROOTUtils.a:
	+cd ROOTUtils; make

Matrix/libMPMMatrix.a:
	+cd Matrix; make

.PHONY: doc
doc:
	doxygen Doxyfile

# cleanup

.PHONY: clean
clean:
	-cd GeneralUtils; make clean
	-cd ROOTUtils; make clean
	-cd Matrix; make clean
	-rm -rf html/ latex/

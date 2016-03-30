# assure correct shell is used
SHELL = /bin/sh

all: libs

.PHONY: libs
libs:
	+cd GeneralUtils; make
	+cd ROOTUtils; make
	+cd Matrix; make

.PHONY: rootutils
rootutils:
	+cd GeneralUtils; make
	+cd ROOTUtils; make

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

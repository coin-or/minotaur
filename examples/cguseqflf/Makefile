# A (simple) Makefile for building cguseqflf

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = -llapack -lblas -lgfortran -ldl

MINOTAUR_INCDIR = ../../build-debug/include/minotaur
MINOTAUR_LIBDIR = ../../build-debug/lib

BOOST_INCDIR = /home/amahajan/projects/minotaur-release/t1/third-party/include

RPATH_FLAGS = -Wl,-rpath=${MINOTAUR_LIBDIR}

.PHONY: default

default: cguseqflf

%: %.cpp
	$(CXX) $(CXXFLAGS) -I$(MINOTAUR_INCDIR) -I$(BOOST_INCDIR) \
	-L$(MINOTAUR_LIBDIR) $@.cpp -o $@ -lminotaur \
	$(RPATH_FLAGS) $(LD_ADD)


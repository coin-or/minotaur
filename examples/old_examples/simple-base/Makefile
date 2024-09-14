# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = -llapack

MINOTAUR_INCDIR = ../../build-debug/include
MINOTAUR_LIBDIR = ../../build-debug/lib

BOOST_INCDIR = ../../minotaur-externals/

RPATH_FLAGS = -Wl,-rpath=${MINOTAUR_LIBDIR}

%: %.cpp
	$(CXX) $(CXXFLAGS) -I$(MINOTAUR_INCDIR) -I$(BOOST_INCDIR) \
	  -L$(MINOTAUR_LIBDIR) $@.cpp -o $@ -lminotaur $(RPATH_FLAGS) $(LD_ADD)



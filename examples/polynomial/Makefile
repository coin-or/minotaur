# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = -llapack -lblas -ldl -lgfortran

MINOTAUR_SRC   = ../../../minotaur
MINOTAUR_BUILD = ../../../minotaur.build

MINOTAUR_INCDIR = $(MINOTAUR_BUILD)/include/minotaur
MINOTAUR_LIBDIR = $(MINOTAUR_BUILD)/lib

THIRD_PARTY = $(MINOTAUR_SRC)/third-party

ASL_DIR      = $(THIRD_PARTY)/asl-20120126
ASL_LIB      = $(THIRD_PARTY)/asl-20120126/amplsolver.a

BOOST_INCDIR = $(THIRD_PARTY)

FILTER_LIB   = $(THIRD_PARTY)/filter-87/filterlib.a

IPOPT_LIBDIR = $(THIRD_PARTY)/ipopt-3.10.2/lib
IPOPT_LIB    = -lipopt -lcoinmumps -lpthread

OSI_LIBDIR = $(THIRD_PARTY)/clp-1.14.6/lib
OSI_LIB    = -lOsiClp -lOsi -lClp -lCoinUtils

EXTRA_LIBDIR   = $(THIRD_PARTY)/lapack-3.4.0/lib

RPATH_FLAGS = -Wl,-rpath=${MINOTAUR_LIBDIR}

.PHONY: default

default: polynomial

%: %.cpp
	$(CXX) $(CXXFLAGS) -I$(MINOTAUR_INCDIR) -I$(BOOST_INCDIR) \
	-I$(ASL_DIR) -L$(MINOTAUR_LIBDIR) -L$(IPOPT_LIBDIR)  -L$(OSI_LIBDIR) -L$(EXTRA_LIBDIR) $@.cpp -o $@ \
	-lmntrengfac -lmntrfiltersqp -lmntrbqpd -lmntripopt -lmntrosilp -lmntrampl -lminotaur \
	${FILTER_LIB} ${ASL_LIB} $(IPOPT_LIB) $(OSI_LIB) $(RPATH_FLAGS) $(LD_ADD)


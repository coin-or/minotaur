# A (simple) Makefile for building minotaur executables on Linux.

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = -lgfortran -llapack -ldl

THIRDPARTY = ../../release/third-party-0.2.1-linux-x86_64
MINOTAUR_INCDIR = ../../build-debug/include/minotaur
MINOTAUR_LIBDIR = ../../build-debug/lib

BOOST_INCDIR = $(THIRDPARTY)/include

ASL_LIB = $(THIRDPARTY)/lib/amplsolver.a
FILTER_LIB = $(THIRDPARTY)/lib/libfiltersqp.a $(THIRDPARTY)/lib/libbqpd.a

RPATH_FLAGS = -Wl,-rpath=${MINOTAUR_LIBDIR}

.PHONY: default

default: bqpd-qp filter-qp

%: %.cpp
	$(CXX) $(CXXFLAGS) -I$(MINOTAUR_INCDIR) -I$(BOOST_INCDIR) \
	-L$(MINOTAUR_LIBDIR) $@.cpp -o $@ -lminotaur \
	-lmntrfiltersqp -lmntrbqpd -lmntrampl \
	${FILTER_LIB} ${ASL_LIB} $(RPATH_FLAGS) $(LD_ADD)


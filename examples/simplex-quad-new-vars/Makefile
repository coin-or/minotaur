# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = -llapack -ldl -lgfortran

MINOTAUR_INCDIR = ../../build/include
MINOTAUR_LIBDIR = ../../build/lib

TP_LIBDIR = ../../third-party/lib

RPATH_FLAGS = -Wl,-rpath=${MINOTAUR_LIBDIR} -Wl,-rpath=${TP_LIBDIR}

%: %.cpp
	$(CXX) $(CXXFLAGS) -I$(MINOTAUR_INCDIR)  \
	  -L$(MINOTAUR_LIBDIR) -L$(TP_LIBDIR) $@.cpp -o $@ -lminotaur -lipopt -lOsiClp -lOsi -lClp -lCoinUtils -lcoinmumps $(TP_LIBDIR)/amplsolver.a  $(TP_LIBDIR)/libbqpd.a  $(TP_LIBDIR)/libfiltersqp.a $(RPATH_FLAGS) $(LD_ADD)



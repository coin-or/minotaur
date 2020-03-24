# Copyright (C) 2009, 2010 Hans Pirnay. 
# All Rights Reserved.
# This code is published under the Eclipse Public License.
#
# Author: Hans Pirnay, 2009-12-15


#############################################################################
# Do not modify anything below unless you know what you're doing.
prefix      = /home/mustafa.vora/minotaur/third-party
exec_prefix = ${prefix}
libdir      = ${exec_prefix}/lib
bindir      = ${exec_prefix}/bin
CYGPATH_W   = echo
SRCDIR      = /home/mustafa.vora/minotaur/third-party/minotaur-0.2.3-third-party-src/ipopt-3.12.13/Ipopt/contrib/sIPOPT/AmplSolver
ipoptsrcdir  = $(SRCDIR)/../../../src
senssrcdir = $(SRCDIR)/../src

CXX         = g++
CXXFLAGS    = -O3 -pipe -DNDEBUG -Wparentheses -Wreturn-type -Wcast-qual -Wall -Wpointer-arith -Wwrite-strings -Wconversion -Wno-unknown-pragmas -Wno-long-long   -DIPOPT_BUILD
LDFLAGS     = $(CXXFLAGS)  -Wl,--rpath -Wl,/home/mustafa.vora/minotaur/third-party/lib

# Include directories (we use the CYGPATH_W variables to allow compilation with Windows compilers)
#   we also need some Ipopt headers that are not installed, so we add $(ipoptsrcdir)/Algorithm and $(ipoptsrcdir)/LinAlg
INCL = `PKG_CONFIG_PATH=/home/mustafa.vora/minotaur/third-party/lib/pkgconfig:/home/mustafa.vora/minotaur/third-party/share/pkgconfig:/home/mustafa.vora/minotaur/third-party/lib64/pkgconfig:/home/mustafa.vora/minotaur/third-party/lib/pkgconfig:/home/mustafa.vora/minotaur/third-party/share/pkgconfig: pkg-config --cflags ipoptamplinterface` -I`$(CYGPATH_W) $(ipoptsrcdir)/Algorithm` -I`$(CYGPATH_W) $(ipoptsrcdir)/LinAlg` -I`$(CYGPATH_W) $(senssrcdir)` $(ADDINCFLAGS)
#INCL = -I`$(CYGPATH_W) /home/mustafa.vora/minotaur/third-party/include/coin`   -I`$(CYGPATH_W) $(ipoptsrcdir)/Algorithm` -I`$(CYGPATH_W) $(ipoptsrcdir)/LinAlg` -I`$(CYGPATH_W) $(senssrcdir)` $(ADDINCFLAGS)

# Linker flags
LIBS = -lsipopt `PKG_CONFIG_PATH=/home/mustafa.vora/minotaur/third-party/lib/pkgconfig:/home/mustafa.vora/minotaur/third-party/share/pkgconfig:/home/mustafa.vora/minotaur/third-party/lib64/pkgconfig:/home/mustafa.vora/minotaur/third-party/lib/pkgconfig:/home/mustafa.vora/minotaur/third-party/share/pkgconfig: pkg-config --libs ipoptamplinterface`
##LIBS = -link -libpath:`$(CYGPATH_W) /home/mustafa.vora/minotaur/third-party/lib` libsipopt.lib libipoptamplinterface.lib libipopt.lib -llapack -lblas -lm  -ldl 
#LIBS = -L/home/mustafa.vora/minotaur/third-party/lib -lsipopt -lipoptamplinterface -lipopt -llapack -lblas -lm  -ldl 

#LIBTOOL = $(SHELL) $(top_builddir)/libtool
#CXXLD = $(CXX)
#CXXLINK = $(LIBTOOL) --tag=CXX --mode=link $(CXXLD) $(AM_CXXFLAGS) \
#	$(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@

sIPOPT_LIB = libsipopt.so

sIPOPT_LIBNAME =$(sIPOPT_LIB).0.0.0

TARGET = ipopt_sens

OBJS   = SensAmplTNLP.o 

VPATH = $(SRCDIR):$(ipoptsrcdir)/Apps/AmplSolver

all: $(OBJS)

install: $(TARGET)
	if test -d $(bindir); then : ; else mkdir $(bindir); fi
	cp $(TARGET) $(bindir)

uninstall:
	rm -f $(bindir)/$(TARGET)

$(TARGET): $(OBJS) ampl_sipopt.cpp
	$(CXX) $(LDFLAGS) $(SRCDIR)/ampl_sipopt.cpp -o $(TARGET) $(OBJS) $(INCL) $(LIBS)

%.o: %.cpp 
	$(CXX) $(CXXFLAGS) $(INCL) -o $@ -c $^

clean:
	rm -f $(OBJS) $(TARGET)

distclean: clean

# Copyright (C) 2003, 2010 International Business Machines and others.
# All Rights Reserved.
# This file is distributed under the Eclipse Public License.

# $Id$

##########################################################################
#    You can modify this example makefile to fit for your own program.   #
#    Usually, you only need to change the five CHANGEME entries below.   #
##########################################################################

# CHANGEME: This should be the name of your executable
EXE = hs071_f

# CHANGEME: Here is the name of all object files corresponding to the source
#           code that you wrote in order to define the problem statement
OBJS = hs071_f.o

# CHANGEME: Additional libraries
ADDLIBS =

# CHANGEME: Additional flags for compilation (e.g., include flags)
ADDINCFLAGS =

##########################################################################
#  Usually, you don't have to change anything below.  Note that if you   #
#  change certain compiler options, you might have to recompile Ipopt.   #
##########################################################################

# Fortran Compiler options
F77 = gfortran

# Fotran Compiler options
FFLAGS = -O3 -pipe  

# additional Fortran Compiler options for linking
F77LINKFLAGS =  -Wl,--rpath -Wl,/home/mustafa.vora/minotaur/third-party/lib

# Include directories (we use the CYGPATH_W variables to allow compilation with Windows compilers)
INCL = `PKG_CONFIG_PATH=/home/mustafa.vora/minotaur/third-party/lib/pkgconfig:/home/mustafa.vora/minotaur/third-party/share/pkgconfig:/home/mustafa.vora/minotaur/third-party/lib64/pkgconfig:/home/mustafa.vora/minotaur/third-party/lib/pkgconfig:/home/mustafa.vora/minotaur/third-party/share/pkgconfig: pkg-config --cflags ipopt` $(ADDINCFLAGS)
#INCL = -I`$(CYGPATH_W) /home/mustafa.vora/minotaur/third-party/include/coin`  $(ADDINCFLAGS)

# Linker flags
LIBS = `PKG_CONFIG_PATH=/home/mustafa.vora/minotaur/third-party/lib/pkgconfig:/home/mustafa.vora/minotaur/third-party/share/pkgconfig:/home/mustafa.vora/minotaur/third-party/lib64/pkgconfig:/home/mustafa.vora/minotaur/third-party/lib/pkgconfig:/home/mustafa.vora/minotaur/third-party/share/pkgconfig: pkg-config --libs ipopt` -lstdc++ -lm
#LIBS = -L/home/mustafa.vora/minotaur/third-party/lib -lipopt -llapack -lblas -lm  -ldl -lstdc++ -lm

# The following is necessary under cygwin, if native compilers are used
CYGPATH_W = echo

all: $(EXE)

.SUFFIXES: .f .o .obj

$(EXE): $(OBJS)
	bla=;\
	for file in $(OBJS); do bla="$$bla `$(CYGPATH_W) $$file`"; done; \
	$(F77) $(F77LINKFLAGS) $(FFLAGS) -o $@ $$bla $(ADDLIBS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS) IPOPT.OUT

.f.o:
	$(F77) $(FFLAGS) $(INCL) -c -o $@ $<


.f.obj:
	$(F77) $(FFLAGS) $(INCL) -c -o $@ `$(CYGPATH_W) '$<'`

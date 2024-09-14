EXAMPLE FOR USING MINOTAUR: OUTER APPROXIMATION (OA)

This directory contains the source code of a MINOTAUR example showing
how to implement outer approximation. The NLP solver is either IPOPT
or FilterSQP. The MILPs are solved directly using MINOTAUR. The solver
is written to work as an AMPL solver.

See also $(MINOTAUR)/src/base/QHHandler.* for an implementation of 
Quesada/Grossmann in MINOTAUR. 

See doxygen documentation at http://mcs.anl.gov/~mahajan/minotaur/html


CONTENT

1. DOWNLOADING MINOTAUR AND OTHER PACKAGES
   1.a. Get MINOTAUR from SVN repository
   1.b. Build MINOTAUR and Externals
   1.c. Simple Test 
2. FILES & SHORT DESCRIPTION
3. IMPLEMENTATION NOTES
   3.a. Makefile
   3.b. Driver: outer-approx.cpp
4. USAGE & TESTING
5. NOTES & DEFICIENCIES


1. DOWNLOADING MINOTAUR AND OTHER PACKAGES

   1.a. Get MINOTAUR from SVN repository

   svn co svn+ssh://leyffer@pbr.cs.wisc.edu/u/l/i/linderot/svn/MINLP/trunk/src/minotaur


   1.b. Build MINOTAUR and Externals

   cd minotaur
   ./scripts/build_with_externals -e minotaur-externals -M . -d build 

   1.b.i Build MINOTAUR and Externals on MacBook Air

   Macs need special compiler options, so here are some special commands.

   cd minotaur
   ./scripts/build_with_externals -e minotaur-externals -M . -d build -b -C -O -s
   
   cd minotaur-externals/minotaur-externals/ipopt-YourVersion/build
   ../configure --disable-shared --enable-static ADD_FFLAGS="-mmacosx-version-min=10.4 -m64"
   make clean; make; make install

   cd minotaur-externals/minotaur-externals/filter-*/src
   add -m64 to Makefile options
   make

   cd build
   copy configure command and add FFLAGS=-m64
   
   make; make install


   1.c. Simple Test 

   ./minotaurampl -=


2. FILES & SHORT DESCRIPTION

   Makefile		makefile that compiles & links OA
   outer-approx.cpp	driver & implementation of OA


3. IMPLEMENTATION NOTES

   3.a. Makefile

	- Set AMPL directory and location of AMPL solver library (amplsolver.a)
	- Set directories and library location for NLP solvers (IPOPT/FilterSQP)
	- Set MINOTAUR directory and location of MINOTAUR library
	
	- Set names of cpp/obj files needed

	- Set C++ flags & Loader flags

	- Compile and link code


   3.b. Driver: outer-approx.cpp


4. USAGE & TESTING


5. NOTES & DEFICIENCIES

   5.a. Using MINOTAUR to solve MILPs is inefficient, because it requires 
	allocate/deallocate for every new MILP relaxation solved. A better 
	way would be to define an MILPEngine derived class in MINOTAUR that
	allows some warm-starting of MILP.



Example created by Sven Leyffer & Ashutosh Mahajan, Argonne, 2011.

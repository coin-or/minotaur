// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2025 The Minotaur Team.
// 

\page stcompile Compile statically linked binaries on Linux

First compile third-party static libraries 
  - cd third-party
  - ./build_third_party -j8 -b -s -d
  - check if the lib directory has the required library files: ipopt, mumps,
    clp, ampl, filtersqp, etc.

Next compile minotaur
  - mkdir minotaur/build-static
  - cd minotaur/build-static
  - ccmake ..
  - set THIRD_PARTY_PATH
  - set MNTR_EXTRA_LIBS to -lcoinmumps
  - set BUILD_SHARED_LIBS to OFF
  - press t to toggle the advanced mode
  - set CMAKE_EXE_LINKER_FLAGS to -static
  - press c to configure, g to generate and q to quit
  - make install -j4

Check the binaries by doing: ldd bin/mbnb. It should say 'not an dynamic executable'.



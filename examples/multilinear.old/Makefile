#
#  A simple (non-automake) Makefile for building a driver
#

CXX = g++
CXXFLAGS = -g

MINOTAUR_INC = -I ../base -I../engines/OsiLP
MINOTAUR_LIB_DIR = ${HOME}/svn-mods/MINLP/src/minotaur/Release/lib
#MINOTAUR_LIB_DIR = ${HOME}/sandbox/build-minotaur/lib 

#AMPL_LIB = ${HOME}/src/amplsolver/build-64/amplsolver.a 
AMPL_LIB = ${HOME}/ampl/solvers/amplsolver.a 
#AMPL_LIB = ${HOME}/src/amplsolver/amplsolver.a 

BLAS_LAPACK_LIB = #-lblas -llapack
BOOST_INC = 
#COIN_INC = -I${HOME}/src/osi-0.100.0/build-64-shared/include/coin

COIN_INC = -I${HOME}/svn-mods/coin-Cbc/Release/debug/include/coin
COIN_LIB = -I${HOME}/svn-mods/coin-Cbc/Release/debug/lib

ML_SRCS := multilinear.cpp\
	MultilinearFormulation.cpp

FT_SRCS := formtest.cpp\
	MultilinearFormulation.cpp

ML_OBJS := ${ML_SRCS:.cpp=.o}
FT_OBJS := ${FT_SRCS:.cpp=.o}

first: formtest

multilinear: ${ML_OBJS}
	g++ -o multilinear ${ML_OBJS} -L${MINOTAUR_LIB_DIR} -lminotaur ${AMPL_LIB} ${BLAS_LAPACK_LIB} ${COIN_LIB} -Wl,-rpath=${MINOTAUR_LIB_DIR} 

formtest: ${FT_OBJS}
	g++ -o formtest ${FT_OBJS} -L${MINOTAUR_LIB_DIR} -lminotaur ${AMPL_LIB} ${BLAS_LAPACK_LIB} ${COIN_LIB} -Wl,-rpath=${MINOTAUR_LIB_DIR} 


.cpp.o:
	${CXX} ${CXXFLAGS} ${MINOTAUR_INC} ${COIN_INC} -c $< -o $@

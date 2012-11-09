# A (simple) Makefile for building minotaur executables.

CXX = g++
CXXFLAGS = -ggdb
LD_ADD = 

AMPL_DIR = /home/mahajan/src/amplsolver/build-64
AMPL_LIB = ${AMPL_DIR}/amplsolver.a
AMPL_INC = -I$(AMPL_DIR)

IPOPT_INC = -I/home/mahajan/sandbox/minotaur-t/build/minotaur-externals/ipopt-3.8.1/build/include/coin/
IPOPT_LIB_DIR = /home/mahajan/sandbox/minotaur-t/build/minotaur-externals/ipopt-3.8.1/build/lib
IPOPT_LIB = -L${IPOPT_LIB_DIR} -lipopt -Wl,-rpath=${IPOPT_LIB_DIR}

MINOTAUR_INC = -I/home/mahajan/sandbox/minotaur/build-debug/include/minotaur
MINOTAUR_LIB_DIR = /home/mahajan/sandbox/minotaur/build-debug/lib

MINOTAUR_LIB = -L${MINOTAUR_LIB_DIR} -lminotaur -lmntrampl -lmntripopt

QUAD_SRCS := quad-socp.cpp lSOC.cpp
QUAD_OBJS := ${QUAD_SRCS:.cpp=.o}

MONOMIAL_SRCS := monomial-socp.cpp Polynomial.cpp lSOC.cpp
MONOMIAL_OBJS := ${MONOMIAL_SRCS:.cpp=.o}

CXXFLAGS += ${MINOTAUR_INC} ${AMPL_INC} ${IPOPT_INC} 
LD_ADD += ${MINOTAUR_LIB} ${AMPL_LIB} -Wl,-rpath=${MINOTAUR_LIB_DIR} ${IPOPT_LIB}

quad-socp: ${QUAD_OBJS} lSOC.h
	${CXX} -o quad-socp ${QUAD_OBJS} ${CXXFLAGS} ${LD_ADD} 

monomial-socp: ${MONOMIAL_OBJS} Polynomial.h
	${CXX} -o monomial-socp ${MONOMIAL_OBJS} ${CXXFLAGS} ${LD_ADD} 

.cpp.o:
	${CXX} ${CXXFLAGS} -c $< -o $@



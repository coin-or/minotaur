# Makefile for building outer approximation in minotaur 

###########################################################################
# PART I: SYSTEM DEPENDENT PATHS AND VARIABLES
# compilers and options
CXX = g++
CXXFLAGS = -gdwarf-2
LD_ADD = -L/usr/local/lib/gcc/i386-apple-darwin9.0.0/4.3.0/x86_64 -L/usr/local/lib/gcc/i386-apple-darwin9.0.0/4.3.0/../../../x86_64 -L/usr/local/lib/gcc/i386-apple-darwin9.0.0/4.3.0 -L/usr/local/lib/gcc/i386-apple-darwin9.0.0/4.3.0/../../.. /usr/local/lib/gcc/i386-apple-darwin9.0.0/4.3.0/x86_64/libgfortranbegin.a /usr/local/lib/x86_64/libgfortran.dylib -lm

BOOST_INCDIR = ../../minotaur-externals/minotaur-externals

# paths to AMPL library and include directory
AMPL_DIR = /Users/leyffer/software/ampl/solvers/OBJ-i386
AMPL_LIB = ${AMPL_DIR}/amplsolver.a
AMPL_INC = -I${AMPL_DIR}

# paths to IPOPT library and include directory
MINOTAUR_DIR = /Users/leyffer/projects/MINLP/src/minotaur/build
MINOTAUR_EXT = /Users/leyffer/projects/MINLP/src/minotaur/minotaur-externals/minotaur-externals
IPOPT_INC = -I/${MINOTAUR_EXT}/ipopt-3.8.3/build/include/coin/
IPOPT_LIB_DIR = ${MINOTAUR_EXT}/ipopt-3.8.3/build/lib
IPOPT_LIB = -L${IPOPT_LIB_DIR} -lipopt -Wl,-rpath=${IPOPT_LIB_DIR}

# paths to FilterSQP library and include directory
FILTER_LIB = ${MINOTAUR_EXT}/filter-82/src/filterlib.a

# paths to OSI library
OSI_LIB_DIR = ${MINOTAUR_EXT}/osi-0.102.1/build/lib
OSI_LIB = -L${OSI_LIB_DIR} -lOsiClp -lOsi -lClp -lCoinUtils

# paths to MINOTAUR library and include directory
MINOTAUR_INC = -I${MINOTAUR_DIR}/include/minotaur
MINOTAUR_LIB_DIR = ${MINOTAUR_DIR}/lib
###########################################################################

MINOTAUR_LIB = -L${MINOTAUR_LIB_DIR} -lminotaur -lmntrampl -lmntrbqpd -lmntrfiltersqp -lmntrosilp

OA_SRCS := outer-approx.cpp utilOA.cpp
OA_OBJS := ${OA_SRCS:.cpp=.o}
GBD_SRCS := benders.cpp utilGBD.cpp
GBD_OBJS := ${GBD_SRCS:.cpp=.o}

CXXFLAGS += ${MINOTAUR_INC} -I$(BOOST_INCDIR) ${AMPL_INC} ${IPOPT_INC} 
LD_ADD += ${MINOTAUR_LIB} ${AMPL_LIB} ${FILTER_LIB} ${OSI_LIB} 

all: outer-approx benders

benders: ${GBD_OBJS} 
	${CXX} -o benders ${GBD_OBJS} ${CXXFLAGS} ${LD_ADD} -lgfortran


outer-approx: ${OA_OBJS} 
	${CXX} -o outer-approx ${OA_OBJS} ${CXXFLAGS} ${LD_ADD}

.cpp.o:
	${CXX} ${CXXFLAGS} -c $< -o $@

clean:
	rm -f *.o outer-approx benders



CC        := g++
CFLAG     := -g -Wall 
HEADER    := ../include
COIN_H    := /sandbox/mahajan/minotaur/minotaur-externals/osi-0.102.1/build/include/coin/
INCLUDE   := -I${HEADER} -I${COIN_H}

all:libpower.a
# ========================================
# include the dependency file
# ========================================
-include .depend
depend: 
	g++ -MM *.cpp ${INCLUDE} > .depend

# ========================================
# define the suffix rules
# ========================================

libpower.a: constraint.o linearfunc.o objective.o problem.o \
	uuid.o variable.o genobj.o sparsevector.o variablehandler.o
	ar rcv $@ $^;
#	rm $^;
%:%.o 
	${CC} ${CFLAG} -o $@ $< ${INCLUDE} ${LIB}


%.o:%.cpp
	${CC} ${CFLAG} -c $< ${INCLUDE};

clean:
	rm -rf *.o libpower.a *~

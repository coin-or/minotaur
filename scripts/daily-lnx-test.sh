#!/bin/bash

## The directory ${TEST_DIR} is deleted everyday and recreated.
## All builds will take place within ${TEST_DIR}
TEST_DIR=/sandbox/mahajan/minotaur-test

## git info
GIT_REPOS="https://github.com/ashutoshmahajan/minotaur"

## LOGS
WEB_DIR=/mcs/web/research/projects/minotaur/nightly/build-log/petsc

## parallel flag
CPUS="8"

## summary file
SUMMARY="summary.log"

## where is doxygen, leave empty if it is already in PATH
DOXYGEN="/home/mahajan/install/doxygen-1.7.1/bin"

## where should doxygen-docs be copied after generation. Last '/' is
## important.
DOXY_DIR="/mcs/web/research/projects/minotaur/nightly/doxygen/"

## where is cmake, leave empty if it is already in PATH
CMAKE="/home/mahajan/install/cmake-2.8.1/bin"

## url where logs are accessible.
URL="http://www.mcs.anl.gov/research/projects/minotaur/nightly/build-log/petsc"

## delimitor
LINE="--------------------------------------------------"
##########################################################################
# end of configurable options
##########################################################################

function doTest {
# empty $NAME is not allowed
if [ "x${NAME}" == "x" ]
then
  echo>&2 "NAME not defined for this test. Exiting."
  exit 1
fi

# empty OPTIONS is allowed
./scripts/build_with_externals -M . -d ./${NAME} -e ./minotaur-externals \
                               -j ${CPUS} -l ${NAME}.log -r ${NAME}.err   \
			       ${OPTIONS}
cd ${NAME}
make utest >> ../${NAME}.log 2>> ../${NAME}.err
make install >> ../${NAME}.log 2>> ../${NAME}.err
cd - >> /dev/null

}

function checkTest {
okMatch=
okMatch=`grep -E -o "OK (.*)" ${NAME}.err`
if [[ x${okMatch} == "x" ]]
then
  echo make utest output for ${NAME}: ERROR >> ${SUMMARY}
else
  echo make utest output for ${NAME}: ${okMatch} >> ${SUMMARY}
fi
}

function listBins {
ls -lt ${NAME}/bin >> ${NAME}.log
ls -lt ${NAME}/lib >> ${NAME}.log
ls ${NAME}/include/minotaur/* >> ${NAME}.log
}

function testFiles {
files_exist=1
for f in ${FILES}
do
  if [ ! -e ${NAME}/${f} ]
  then
    files_exist=0
    break
  fi
done

if [ ${files_exist} -eq 1 ]
then
  echo "compiling ${NAME}: OK." >> ${SUMMARY}
else
  echo "compiling ${NAME}: ERROR. ${f} not built." >> ${SUMMARY}
fi
}

function addDoxygen {
if [[ x${DOXYGEN} == "x" ]]
then
  echo "Not using any user specified doxygen installation"
else
  export PATH=${DOXYGEN}:${PATH}
fi
}

function addCmake {
if [[ x${CMAKE} == "x" ]]
then
  echo "Not using any user specified cmake installation"
else
  export PATH=${CMAKE}:${PATH}
fi
}

##########################################################################
# Main
##########################################################################

echo "Minotaur daily test report"
date
hostname -f
uname -a
echo "Detailed log available at " \
     "${URL}"

# remove previous files
rm -rf ${TEST_DIR}

# add doxygen to path
addDoxygen

# add cmake to path
addCmake

# get latest version
git clone ${GIT_REPOS} ${TEST_DIR} &> git.log

echo ""
if [ -d ${TEST_DIR}/src ]
then
  echo "Minotaur src directory checked out."
else
  echo>&2 "Error in cloning with git!"
  exit 1
fi

mv git.log ${TEST_DIR}/
cd ${TEST_DIR}
echo "Minotaur version: `git describe`"
NAME=
OPTIONS=
##########################################################################
## TEST 1
## Build all externals
##########################################################################
NAME=all_externals
./scripts/build_externals -d . -j 8 -l ${NAME}.log -r ${NAME}.err

##########################################################################
## TEST 2
## Build Minotaur base alone
##########################################################################
NAME=build-base
OPTIONS="-fioa"
FILES="lib/libminotaur.so"
doTest; testFiles; checkTest

## debug
NAME=build-base-debug
OPTIONS=-fioab
doTest; testFiles; checkTest

## static
NAME=build-base-static
OPTIONS=-fioas
FILES="lib/libminotaur.a"
doTest; testFiles; checkTest

##########################################################################
## TEST 3
## Build Minotaur with filter alone
##########################################################################
NAME=build-filter
OPTIONS=-ioa
FILES="lib/libminotaur.so lib/libmntrbqpd.a lib/libmntrfiltersqp.so"
doTest; testFiles; checkTest

## debug
NAME=build-filter-debug
OPTIONS=-ioab
doTest; testFiles; checkTest

## static
NAME=build-filter-static
OPTIONS=-ioas
FILES="lib/libminotaur.a lib/libmntrbqpd.a lib/libmntrfiltersqp.a"
doTest; testFiles; checkTest

##########################################################################
## TEST 4
## Build Minotaur with ipopt alone
##########################################################################
NAME=build-ipopt
OPTIONS=-foa
FILES="lib/libminotaur.so lib/libmntripopt.so"
doTest; testFiles; checkTest

## debug
NAME=build-ipopt-debug
OPTIONS=-foab
doTest; testFiles; checkTest

## static
NAME=build-ipopt-static
OPTIONS=-foas
FILES="lib/libminotaur.a lib/libmntripopt.a"
doTest; testFiles; checkTest

##########################################################################
## TEST 5
## Build Minotaur with osi alone
##########################################################################
NAME=build-osi
OPTIONS=-ifa
FILES="lib/libminotaur.so lib/libmntrosilp.so"
doTest; testFiles; checkTest

## debug
NAME=build-osi-debug
OPTIONS=-ifab
doTest; testFiles; checkTest

## static
NAME=build-osi-static
OPTIONS=-ifas
FILES="lib/libminotaur.a lib/libmntrosilp.a"
doTest; testFiles; checkTest

##########################################################################
## TEST 6
## Build Minotaur with ampl alone
##########################################################################
NAME=build-ampl
OPTIONS=-ifo
FILES="lib/libminotaur.so lib/libmntrampl.so"
doTest; listBins; testFiles; checkTest

## debug
NAME=build-ampl-debug
OPTIONS=-ifob
doTest; listBins; testFiles; checkTest

## static
NAME=build-ampl-static
OPTIONS=-ifos
FILES="lib/libminotaur.a lib/libmntrampl.a"
doTest; listBins; testFiles; checkTest

##########################################################################
## TEST 7
## Build Minotaur with ampl + osi + ipopt + filter 
##########################################################################
NAME=build-all
OPTIONS=
FILES="lib/libminotaur.so lib/libmntrbqpd.a lib/libmntrampl.so lib/libmntrfiltersqp.so lib/libmntripopt.so lib/libmntrosilp.so"
doTest; listBins; testFiles; checkTest

## debug
NAME=build-all-debug
OPTIONS=-b
doTest; listBins; testFiles; checkTest

## valgrind
cd ${NAME}/src/testing
valgrind --leak-check=full --show-reachable=yes \
./unittest all >> ../../../${NAME}.out 2>> ../../../${NAME}.err
cd - >> /dev/null

## static
NAME=build-all-static
OPTIONS=-s
FILES="lib/libminotaur.a lib/libmntrampl.a lib/libmntrbqpd.a lib/libmntrfiltersqp.a lib/libmntripopt.a  lib/libmntrosilp.a"
doTest; listBins; testFiles; checkTest

##########################################################################
## Test manual build without automake
##########################################################################
## we build with -j 1, otherwise it doesn't build sometimes.
NAME=build-all-manual
OPTIONS=-x
./scripts/build_with_externals -M . -d ./${NAME} -e ./minotaur-externals \
                               -j 1 -l ${NAME}.log -r ${NAME}.err  \
			       ${OPTIONS}
ls -lt ${NAME} >> ${NAME}.log
FILES="lib/libminotaur.a lib/libmntrbqpd.a lib/libmntrampl.a lib/libmntrfiltersqp.a lib/libmntripopt.a lib/libmntrosilp.a bin/bnb bin/glob bin/qg bin/qpd"
testFiles

##########################################################################
## Copy logs to web
##########################################################################

cd build-all
make doc >> ../doc.log 2>> ../doc.err
rm -rf ${DOXY_DIR}
rsync -a ${TEST_DIR}/build-all/doxygen/html/ ${DOXY_DIR}
cd - >> /dev/null

rm -rf ${WEB_DIR}
mkdir ${WEB_DIR}
cp -t ${WEB_DIR}/ *.log *.err

echo ""
echo Summary
echo ${LINE}
cat ${SUMMARY}

echo ""
echo "Errors:"
echo ${LINE}

for f in ${WEB_DIR}/build-*err
do
	echo " "
	echo Errors in `basename ${f} .err`:
	echo ${LINE}
	cat ${f}
done

echo ""
echo "End of tests"
date
echo ${LINE}


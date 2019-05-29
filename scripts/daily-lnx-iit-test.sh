#!/bin/bash

#
# Script for testing compilation and unit tests of Minotaur libraries. Main
# tasks:
# 1. Third party compilation
# 2. Minotaur compilation under various flags
# 3. Unit tests
# We assume that gfortran is installed on the system, but blas and lapack are
# not. Blas and Lapack are compiled as third-party libs. Ipopt, Clp, etc are
# linked to these third-party libs
# 
# Does not use build_with_externals script
#

## The directory ${TEST_DIR} is deleted everyday and recreated.
## All builds will take place within ${TEST_DIR}
TEST_DIR=/home/amahajan/tmp/minotaur-test
TP_DIR=${TEST_DIR}/third-party

## git info
GIT_REPOS="https://github.com/minotaur-solver/minotaur"

## Local LOGS
WEB_DIR=/home/amahajan/misc/web/minotaur/nightly/origin
REM_WEB_DIR=powai:/home/amahajan/webpage/minotaur/nightly/origin

## parallel flag
CPUS="8"

## summary file
SUMMARY="summary.log"

## where is doxygen, leave empty if it is already in PATH
DOXYGEN=""

## where is cmake, leave empty if it is already in PATH
CMAKE=""

## url where logs can be viewed.
URL="http://www.ieor.iitb.ac.in/files/faculty/amahajan/minotaur/nightly/origin/index.html"

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

mkdir ${NAME}
cd ${NAME}
cmake ${CARGS} ${TEST_DIR} >> ../${NAME}.log 2>> ../${NAME}.err
make -j $CPUS utest >> ../${NAME}.log 2>> ../${NAME}.err
make -j $CPUS install >> ../${NAME}.log 2>> ../${NAME}.err
cd - >> /dev/null

}

function checkTest {
ok_match=
ok_match=`grep -E -o "OK (.*)" ${NAME}.err`
err_cnt=`grep -c "error:" ${NAME}.err`
war_cnt=`grep "warning:" ${NAME}.err | grep -c -v third-party`
if [[ x${ok_match} == "x" ]]
then
  echo ${NAME}: utest ERROR >> ${SUMMARY}
else
  echo ${NAME}: ${err_cnt} errors and ${war_cnt} warnings in compiling, utest output: ${ok_match} >> ${SUMMARY}
  echo "" >> ${SUMMARY}
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
  echo "${NAME}: OK, all binaries compiled." >> ${SUMMARY}
else
  echo "${NAME}: ERROR, binary ${f} not built." >> ${SUMMARY}
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
##########################################################################
## TEST 1
## Build third-party
##########################################################################
NAME=third-party
cd ${TP_DIR}
./build_third_party -j 8 -l ${NAME}.log -r ${NAME}.err 
cd ${TEST_DIR}


##########################################################################
## TEST 2
## Build Minotaur base alone
##########################################################################
NAME=build-base
FILES="lib/libminotaur.so"
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
doTest; testFiles; checkTest

## debug
NAME=build-base-debug
FILES="lib/libminotaur.so"
CARGS+=" -DCMAKE_BUILD_TYPE:String=Debug"
doTest; testFiles; checkTest

## static+debug
NAME=build-base-static-debug
FILES="lib/libminotaur.a"
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; testFiles; checkTest

## static
NAME=build-base-static
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; testFiles; checkTest

##########################################################################
## TEST 3
## Build Minotaur with filter alone
##########################################################################
NAME=build-filter
FILES="lib/libminotaur.so lib/libmntrbqpd.a lib/libmntrfiltersqp.so"
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DBQPD_LIB_DIR:PATH=${TD_DIR}/lib"  
CARGS+=" -DFILTER_LIB_DIR:PATH=${TD_DIR}/lib"
doTest; testFiles; checkTest

## debug
NAME=build-filter-debug
FILES="lib/libminotaur.so lib/libmntrbqpd.a lib/libmntrfiltersqp.so"
CARGS+=" -DCMAKE_BUILD_TYPE:String=Debug"
doTest; testFiles; checkTest

## static+debug
NAME=build-filter-static-debug
FILES="lib/libminotaur.a lib/libmntrbqpd.a lib/libmntrfiltersqp.a"
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; testFiles; checkTest

## static
NAME=build-filter-static
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DBQPD_LIB_DIR:PATH=${TD_DIR}/lib"  
CARGS+=" -DFILTER_LIB_DIR:PATH=${TD_DIR}/lib"
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; testFiles; checkTest

##########################################################################
## TEST 4
## Build Minotaur with ipopt alone
##########################################################################
NAME=build-ipopt
FILES="lib/libminotaur.so lib/libmntripopt.so"
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DIPOPT_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DIPOPT_LIB_DIR:PATH=${TP_DIR}/lib" 
doTest; testFiles; checkTest

## debug
NAME=build-ipopt-debug
CARGS+=" -DCMAKE_BUILD_TYPE:String=Debug"
doTest; testFiles; checkTest

## debug+static
NAME=build-ipopt-static-debug
FILES="lib/libminotaur.a lib/libmntripopt.a"
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; testFiles; checkTest

## static
NAME=build-ipopt-static
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DIPOPT_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DIPOPT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; testFiles; checkTest

##########################################################################
## TEST 5
## Build Minotaur with osi alone
##########################################################################
NAME=build-osi
FILES="lib/libminotaur.so lib/libmntrosilp.so"
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DOSI_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DOSI_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DOSICLP:BOOL=ON"
doTest; testFiles; checkTest

## debug
NAME=build-osi-debug
CARGS+=" -DCMAKE_BUILD_TYPE:String=Debug"
doTest; testFiles; checkTest

## static+debug
NAME=build-osi-static-debug
FILES="lib/libminotaur.a lib/libmntrosilp.a"
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; testFiles; checkTest

## static
NAME=build-osi-static
FILES="lib/libminotaur.a lib/libmntrosilp.a"
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DOSI_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DOSI_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DOSICLP:BOOL=ON"
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; testFiles; checkTest

##########################################################################
## TEST 6
## Build Minotaur with Osi+Cbc alone
##########################################################################
NAME=build-cbc
FILES="lib/libminotaur.so lib/libmntrosilp.so lib/libmntrcbc.so"
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DOSI_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DOSI_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DOSICLP:BOOL=ON"
CARGS+=" -DCBC_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DCBC_LIB_DIR:PATH=${TP_DIR}/lib" 
doTest; testFiles; checkTest

## debug
NAME=build-cbc-debug
FILES="lib/libminotaur.so lib/libmntrosilp.so lib/libmntrcbc.so"
CARGS+=" -DCMAKE_BUILD_TYPE:String=Debug"
doTest; testFiles; checkTest

## static+debug
NAME=build-cbc-static-debug
FILES="lib/libminotaur.a lib/libmntrosilp.a lib/libmntrcbc.a"
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; testFiles; checkTest

## static
NAME=build-cbc-static
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DOSI_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DOSI_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DOSICLP:BOOL=ON"
CARGS+=" -DCBC_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DCBC_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; testFiles; checkTest

##########################################################################
## TEST 7
## Build Minotaur with ampl alone
##########################################################################
NAME=build-ampl
FILES="lib/libminotaur.so lib/libmntrampl.so"
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DASL_INC_DIR:PATH=${TP_DIR}/include/asl" 
CARGS+=" -DASL_LIB_DIR:PATH=${TP_DIR}/lib" 
doTest; listBins; testFiles; checkTest

## debug
NAME=build-ampl-debug
CARGS+=" -DCMAKE_BUILD_TYPE:String=Debug"
doTest; listBins; testFiles; checkTest

## static+debug
NAME=build-ampl-static-debug
FILES="lib/libminotaur.a lib/libmntrampl.a"
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; listBins; testFiles; checkTest

## static
NAME=build-ampl-static
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DASL_INC_DIR:PATH=${TP_DIR}/include/asl" 
CARGS+=" -DASL_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; listBins; testFiles; checkTest

##########################################################################
## TEST 8
## Build Minotaur with ampl + osi + cbc + ipopt + filter 
##########################################################################
NAME=build-all
FILES="lib/libminotaur.so lib/libmntrbqpd.a lib/libmntrampl.so lib/libmntrfiltersqp.so lib/libmntripopt.so lib/libmntrosilp.so lib/libmntrcbc.so bin/bnb bin/glob bin/qg bin/qpd"
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DASL_INC_DIR:PATH=${TP_DIR}/include/asl" 
CARGS+=" -DASL_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DCBC_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DCBC_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DOSI_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DOSI_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DOSICLP:BOOL=ON"
CARGS+=" -DIPOPT_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DIPOPT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DBQPD_LIB_DIR:PATH=${TD_DIR}/lib"  
CARGS+=" -DFILTER_LIB_DIR:PATH=${TD_DIR}/lib"
doTest; listBins; testFiles; checkTest

## debug
NAME=build-all-debug
CARGS+=" -DCMAKE_BUILD_TYPE:String=Debug"
doTest; listBins; testFiles; checkTest

## valgrind
cd ${NAME}/src/testing
valgrind --leak-check=full --show-reachable=yes \
./unittest all >> ../../../${NAME}.out 2>> ../../../${NAME}.err
cd - >> /dev/null

## static+debug
NAME=build-all-static-debug
FILES="lib/libminotaur.a lib/libmntrbqpd.a lib/libmntrampl.a lib/libmntrfiltersqp.a lib/libmntripopt.a lib/libmntrosilp.a lib/libmntrcbc.a bin/bnb bin/glob bin/qg bin/qpd"
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; listBins; testFiles; checkTest

## static
NAME=build-all-static
CARGS="-DBOOST_INC_DIR:PATH=${TP_DIR}/include "
CARGS+=" -DCPPUNIT_INC_DIR:PATH=${TP_DIR}/include/"
CARGS+=" -DCPPUNIT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DASL_INC_DIR:PATH=${TP_DIR}/include/asl" 
CARGS+=" -DASL_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DCBC_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DCBC_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DOSI_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DOSI_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DOSICLP:BOOL=ON"
CARGS+=" -DIPOPT_INC_DIR:PATH=${TP_DIR}/include" 
CARGS+=" -DIPOPT_LIB_DIR:PATH=${TP_DIR}/lib" 
CARGS+=" -DBQPD_LIB_DIR:PATH=${TD_DIR}/lib"  
CARGS+=" -DFILTER_LIB_DIR:PATH=${TD_DIR}/lib"
CARGS+=" -DBUILD_SHARED_LIBS:BOOL=0"
doTest; listBins; testFiles; checkTest

## static+spew
NAME=build-all-static-spew
CARGS+=" SPEW_FLAG:BOOL=ON"
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
rsync -a --delete ${TEST_DIR}/build-all/doxygen/html/ ${REM_WEB_DIR}/html/
cd - >> /dev/null

rm -rf ${WEB_DIR}
mkdir ${WEB_DIR}
mkdir ${WEB_DIR}/build-log
cp -t ${WEB_DIR}/build-log/ *.log *.err
cd ${WEB_DIR}
tar -zcf build-log.tar.gz build-log

rsync -a ${WEB_DIR}/build-log.tar.gz ${REM_WEB_DIR}/

echo ""
echo Summary
echo ${LINE}
cat ${SUMMARY}

date
echo ${LINE}


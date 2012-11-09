#!/bin/bash

##############################################################################
# Change the last year in copyright to XXXX
# e.g. ./change_copyright 2010
# will change any line like
# (C)opyright 2008 -- 2009 The MINOTAUR Team
# to
# (C)opyright 2008 -- 2010 The MINOTAUR Team

# The script actually changes any thing after the first year, 
# (C)opyright 2056yeth adsj d f aksd dakjsfk
# to
# (C)opyright 2056 -- 2010 The MINOTAUR Team
# so beware.
##############################################################################

ARG_STRING=${1}

if [ "x${ARG_STRING}" == "x" ]
then
  echo "usage: ./change_copyright.sh YEAR"
  echo "where YEAR is the current year, e.g., 2010"
  exit 1
fi

NEW_STRING=" - ${ARG_STRING} The MINOTAUR Team."

GREP_QUERY='(C)opyright[[:space:]][0-9]\{4\}'

for f in ../src/engines/*/*.h ../src/engines/*/*.cpp ../src/engines/*/*.f \
	 ../src/engines/*.h ../src/engines/*.cpp \
	 ../src/base/*.h ../src/base/*.cpp \
         ../src/testing/*.cpp ../src/testing/*.h  \
	 ../src/interfaces/ampl/*cpp ../src/interfaces/ampl/*.h
do
  # first check if we have the required line in the file
  if grep -q -G ${GREP_QUERY} ${f}
  then
    echo "changing ${f}"
    sed -i "s/\(${GREP_QUERY}\).*/\1${NEW_STRING}/" ${f}
  else
    echo ${f} does not have copyright information!
  fi
done


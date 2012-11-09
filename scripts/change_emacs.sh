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

NEW_STRING="
// Local Variables:
\n// mode: c++
\n// eval: (c-set-style \"k&r\")
\n// eval: (c-set-offset 'innamespace 0)
\n// eval: (setq c-basic-offset 2)
\n// eval: (setq fill-column 78)
\n// eval: (auto-fill-mode 1)
\n// eval: (setq column-number-mode 1)
\n// eval: (setq indent-tabs-mode nil)
\n// End:"

GREP_QUERY='\/\/[[:space:]]Local[[:space:]]Variables:[[:space:]]*'

for f in ../src/algorithms/*.cpp     ../src/algorithms/*.h \
         ../src/base/*.h             ../src/base/*.cpp \
         ../src/base/linalg/*.h      ../src/base/linalg/*.cpp \
         ../src/engines/*.cpp        ../src/engines/*.h \
         ../src/engines/*/*.h        ../src/engines/*/*.cpp \
         ../src/interfaces/ampl/*cpp ../src/interfaces/ampl/*.h \
         ../src/testing/*.cpp        ../src/testing/*.h 
#for f in Environment.h
do
  # first check if we have the required line in the file
  echo $f
  if grep -i -q -G ${GREP_QUERY} ${f}
  then
    echo "changing ${f}"
    # delete the first line matching GREP_QUERY and also everything after.
    sed -n -i.bak -e "/${GREP_QUERY}/I!p" -e "/${GREP_QUERY}/Iq" ${f}
  else
    echo ${f} does not have Local Variables, adding them.
  fi
  echo -e ${NEW_STRING} >> ${f}
done


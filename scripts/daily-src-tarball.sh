#!/bin/bash

## The directory ${SRC_DIR} is deleted everyday and recreated.
SRC_DIR=/sandbox/mahajan/minotaur

WEB_DIR=/mcs/web/research/projects/minotaur/dist/

## svn info
USERNAME=ashu
SVN_REPOS="https://repo.anl-external.org/repos/minotaur"

## delimitor
LINE="--------------------------------------------------"

SVN_VERSION="0"


echo "Creating Minotaur daily source tarball" 
date 

# remove previous files
rm -rf ${SRC_DIR}

# get latest version
svn co ${SVN_REPOS} ${SRC_DIR} 

echo "" 
if [ -d ${SRC_DIR}/src ]
then
  echo "Minotaur src directory checked out."
else
  echo>&2 "Error checking out svn!"
  exit 1
fi

cd ${SRC_DIR}
SVN_VERSION=`svnversion`
echo svn version is ${SVN_VERSION}

sed -i s/\#cmake.*/\#define\ MINOTAUR_SVN_VERSION\ ${SVN_VERSION}/g src/base/Version.h.cmake

find -depth -type d -iname '.svn' -exec rm -rfv '{}' \;

cd ..
tar -zcvf minotaur-nightly-src.tar.gz minotaur

cp minotaur-nightly-src.tar.gz ${WEB_DIR}


#!/bin/bash

## The directory ${SRC_DIR} is deleted everyday and recreated.
SRC_DIR=/sandbox/mahajan/minotaur

WEB_DIR=/mcs/web/research/projects/minotaur/dist/

## git info
GIT_REPOS="https://github.com/ashutoshmahajan/minotaur"

## delimitor
LINE="--------------------------------------------------"

GIT_VERSION="0"


echo "Creating Minotaur daily source tarball" 
date 

# remove previous files
rm -rf ${SRC_DIR}

# get latest version
git clone ${GIT_REPOS} ${SRC_DIR} &> git.log

echo "" 
if [ -d ${SRC_DIR}/src ]
then
  echo "Minotaur src directory checked out."
else
  echo>&2 "Error in cloning with git!"
  exit 1
fi

cd ${SRC_DIR}
GIT_VERSION=`git rev-parse HEAD`
echo current git version is ${GIT_VERSION}

sed -i s/\#cmake.*/\#define\ MINOTAUR_GIT_VERSION\ ${GIT_VERSION}/g src/base/Version.h.cmake

find -depth -type d -iname '.git' -exec rm -rfv '{}' \;

cd ..
tar -zcf minotaur-nightly-src.tar.gz minotaur

cp minotaur-nightly-src.tar.gz ${WEB_DIR}


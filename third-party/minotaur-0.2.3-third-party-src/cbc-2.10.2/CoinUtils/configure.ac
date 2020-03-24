# Copyright (C) 2006, 2009 International Business Machines.
# All Rights Reserved.
# This file is distributed under the Eclipse Public License.

## $Id$

# Author:  Andreas Waechter            IBM    2006-04-13

#############################################################################
#                       Names and other basic things                        #
#############################################################################

AC_PREREQ(2.59)

AC_INIT([CoinUtils],[2.11.2],[coinutils@lists.coin-or.org])

AC_COPYRIGHT([
Copyright 2006, 2009 International Business Machines and others.
All Rights Reserved.
This file is part of the open source package Coin which is distributed
under the Eclipse Public License.])

AC_REVISION(0.9)

# List one file in the package so that the configure script can test
# whether the package is actually there
AC_CONFIG_SRCDIR(configure.ac)

# Where should everything be installed by default?  Here, we want it
# to be installed directly in 'bin', 'lib', 'include' subdirectories
# of the directory where configure is run.  The default would be
# /usr/local.
AC_PREFIX_DEFAULT([`pwd`])

#############################################################################
#  Do the tests necessary to configure compilers and initialise autotools   #
#############################################################################

AC_COIN_CREATE_LIBTOOL

#############################################################################
#         Check which subprojects might need to be configured               #
#############################################################################

AC_COIN_MAIN_PACKAGEDIR(Glpk,ThirdParty,[glpk/src/glplpx01.c])
AC_COIN_MAIN_PACKAGEDIR(Blas,ThirdParty,[daxpy.f])
AC_COIN_MAIN_PACKAGEDIR(Lapack,ThirdParty,[LAPACK/SRC/dlarf.f])
AC_COIN_MAIN_PACKAGEDIR(Sample,Data)
AC_COIN_MAIN_PACKAGEDIR(Netlib,Data)
AC_COIN_MAIN_PACKAGEDIR(CoinUtils)

#############################################################################
#                  Check for doxygen                                        #
#############################################################################

AC_COIN_DOXYGEN()

##############################################################################
#                   Finishing up by writing all the output                   #
##############################################################################

# Here list all the files that configure should create (except for the
# configuration header file)
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([doxydoc/doxygen.conf])

# Finally, we let configure write all the output...
AC_COIN_FINALIZE

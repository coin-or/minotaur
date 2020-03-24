# CoinUtils

CoinUtils (*Coin*-OR *Util*itie*s*) is an open-source collection of classes and functions that are generally useful to more than one COIN-OR project.
These utilities include:
 * classes for storing and manipulating sparse matrices and vectors,
 * performing matrix factorization,
 * parsing input files in standard formats, e.g. MPS,
 * building representations of mathematical programs,
 * performing simple presolve operations,
 * warm starting algorithms for mathematical programs, and
 * comparing floating point numbers with a tolerance, among others.

CoinUtils is written in C++ and is released as open source code under the [Eclipse Public License (EPL)](http://www.opensource.org/licenses/eclipse-1.0).
It is available from the [COIN-OR initiative](http://www.coin-or.org/).

The CoinUtils website is https://github.com/coin-or/CoinUtils.

## Getting Started using CoinBrew

To build CoinUtils from source, obtain the `coinbrew` script from
https://coin-or.github.io/coinbrew/
and run


    /path/to/coinbrew fetch --main-proj=CoinUtils
    /path/to/coinbrew build --main-proj=CoinUtils --test
    /path/to/coinbrew install --main-proj=CoinUtils


The `coinbrew` script will fetch [these](Dependencies) additional projects.


## Getting Started without CoinBrew (Expert users)

 1. Obtain the source code, e.g., from https://github.com/coin-or/CoinUtils
 2. Run `./configure -C` to generate makefiles
 3. Run `make` to build the CoinUtils library
 4. Run `make test` to build and run the CoinUtils unit test program
 5. Run `make install` to install library and header files.



## Doxygen Documentation

If you have `Doxygen` available, you can build a HTML documentation by typing

 `make doxydoc` 

in the build directory.
If CoinUtils was build via `coinbrew`, then the build directory is `./build/CoinUtils`.
The doxygen documentation main file is found at `./doxydoc/html/index.html` in the build directory.

If `Doxygen` is not available, you can use also use [this link](http://www.coin-or.org/Doxygen/CoinUtils).


## Project Links

 * [COIN-OR Initiative](http://www.coin-or.org/)
 * [mailing list](http://list.coin-or.org/mailman/listinfo/coinutils)
 * [Report a bug](https://github.com/coin-or/CoinUtils/issues)
 * [Doxygen-generated html documentation](http://www.coin-or.org/Doxygen/CoinUtils) 

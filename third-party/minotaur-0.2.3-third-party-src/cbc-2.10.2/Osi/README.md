# OSI

Osi (*O*pen *S*olver *I*nterface) provides an abstract base class to a generic linear programming (LP) solver, along with derived classes for specific solvers.
Many applications may be able to use the Osi to insulate themselves from a specific LP solver.
That is, programs written to the OSI standard may be linked to any solver with an OSI interface and should produce correct results.
The OSI has been significantly extended compared to its first incarnation.
Currently, the OSI supports linear programming solvers and has rudimentary support for integer programming.
Among others the following operations are supported:
 * creating the LP formulation;
 * directly modifying the formulation by adding rows/columns;
 * modifying the formulation by adding cutting planes provided by [CGL](https://www.github.com/coin-or/Cgl);
 * solving the formulation (and resolving after modifications);
 * extracting solution information;
 * invoking the underlying solver's branch-and-bound component.

The following is a list of derived Osi classes:

|Solver|Derived Class|Note|
|------|-------------|----|
|[Cbc](https://www.github.com/coin-or/Cbc)|OsiCbc| unmaintained | 
|[Clp](https://www.github.com/coin-or/Clp)|OsiClp| |
|[CPLEX](https://www.ibm.com/analytics/cplex-optimizer)|OsiCpx| |
|[DyLP](https://www.github.com/coin-or/DyLP)|OsiDylp| |
|[GLPK](http://www.gnu.org/software/glpk/glpk.html)|OsiGlpk| Glpk <= 4.48 |
|[Gurobi](http://www.gurobi.com)|OsiGrb| |
|[HiGHS](https://www.github.com/coin-or/HiGHS)|OsiHiGHS| under development |
|[MOSEK](http://www.mosek.com)|OsiMsk| |
|[SoPlex](http://soplex.zib.de)|OsiSpx| SoPlex < 4.0 |
|[SYMPHONY](https://www.github.com/coin-or/SYMPHONY)|OsiSym| |
|[Vol](https://www.github.com/coin-or/Vol)|OsiVol| |
|[XPRESS-MP](https://www.fico.com/en/products/fico-xpress-optimization)|OsiXpr| |

Each solver interface is in a separate directory of Osi or distributed
with the solver itself.

Osi is written in C++ and is released as open source code under the [Eclipse Public License (EPL)](http://www.opensource.org/licenses/eclipse-1.0).
It is available from the [COIN-OR initiative](http://www.coin-or.org/).
Within COIN-OR, Osi is used by [Cgl](https://www.github.com/coin-or/Cgl), [Cbc](https://www.github.com/coin-or/Cbc), and [Bcp](https://www.github.com/coin-or/Bcp), among others.

The Osi website is https://github.com/coin-or/Osi.

The main project managers are Lou Hafer (@LouHafer) and Matt Saltzmann (@mjsaltzman).

An incomplete list of recent changes to Osi are found in the [CHANGELOG](Osi/CHANGELOG)

## Getting Started using CoinBrew

To build CoinUtils from source, obtain the `coinbrew` script from
https://coin-or.github.io/coinbrew/
and run


    /path/to/coinbrew fetch --main-proj=Osi
    /path/to/coinbrew build --main-proj=Osi --test
    /path/to/coinbrew install --main-proj=Osi


The `coinbrew` script will fetch [these](Dependencies) additional projects.


## Getting Started without CoinBrew (Expert users)

 0. Install [CoinUtils](https://www.github.com/coin-or/CoinUtils)
 1. Obtain the source code, e.g., from https://github.com/coin-or/Osi
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
 * [mailing list](http://list.coin-or.org/mailman/listinfo/osi)
 * [Report a bug](https://github.com/coin-or/Osi/issues/new)
 * [Doxygen-generated html documentation](http://www.coin-or.org/Doxygen/Osi/hierarchy.html)
 * [OSI2 Discussion](https://projects.coin-or.org/Osi2/wiki/Osi2Discussion)
 * The most recent tutorial on OSI can be accessed from the [page on presentations from the 2004 CORS/INFORMS Joint Meeting in Banff](http://www.coin-or.org/Presentations/CORSINFORMSWorkshop04/index.html).
 * [The COIN-OR Open Solver Interface: Technology Overview](http://www.coin-or.org/Presentations/CORS2004-OSI.pdf): An overview of the COIN-OR OSI and design issues for a next-generation version given at CORS/INFORMS 2004 by Matthew Saltzman.

-------

## Dynamically loading commercial solver libraries

### At build time

It is possible to create an osi build that supports cplex, gurobi and xpress even if you don't have (yet) any of these solvers on your machine using [lazylpsolverlibs](https://code.google.com/p/lazylpsolverlibs/). To do so, follow these steps:

 1. Install lazylpsolverlibs (follow the instructions of the [lazylpsolverlibs wiki](https://code.google.com/p/lazylpsolverlibs/wiki/HowToSetup))
 2. Use the following command line to configure Osi:
```
./configure --with-cplex-incdir="$(pkg-config --variable=includedir lazycplex)/lazylpsolverlibs/ilcplex" \
            --with-cplex-lib="$(pkg-config --libs lazycplex)" \
            --with-gurobi-incdir="$(pkg-config --variable=includedir lazygurobi)/lazylpsolverlibs" \
            --with-gurobi-lib="$(pkg-config --libs lazygurobi)" \
            --with-xpress-incdir="$(pkg-config --variable=includedir lazyxprs)/lazylpsolverlibs" \
            --with-xpress-lib="$(pkg-config --libs lazyxprs)"
```
 3. Then follow the normal installation process (make, make install)

### At run time

Your build should now support cplex, gurobi and xpress, which means that if you install one of these solvers, osi will be able to use it.
At run time, you just need to point one of the environment variables LAZYLPSOLVERLIBS_GUROBI_LIB, LAZYLPSOLVERLIBS_CPLEX_LIB or LAZYLPSOLVERLIBS_XPRS_LIB to the full path of the corresponding solver library.
For example:
```
export LAZYLPSOLVERLIBS_CPLEX_LIB=/usr/ilog/cplex121/bin/x86_debian4.0_4.1/libcplex121.so
```

### Troubleshooting

If pkg-config reports errors during the configure step, try modifying the PKG_CONFIG_PATH variable. Most likely, you need to do:
```
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
```

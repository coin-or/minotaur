# Cgl

The COIN-OR Cut Generation Library (`Cgl`) is a collection of cut generators that can be used with other COIN-OR packages that make use of cuts, such as, among others, the linear solver [Clp](https://github.com/coin-or/Clp) or the mixed integer linear programming solvers [Cbc](https://github.com/coin-or/Cbc) or [BCP](https://github.com/coin-or/Bcp).
`Cgl` uses the abstract class `OsiSolverInterface` (see [Osi](https://github.com/coin-or/Osi)) to use or communicate with a solver.
It does not directly call a solver.

`Cgl` is written in C++ and is released as open source code under the [Eclipse Public License (EPL)](http://www.opensource.org/licenses/eclipse-1.0).
It is available from the [COIN-OR initiative](http://www.coin-or.org/).

Each cut generator is in a separate directory with its own maintainer.
All generators are combined in one library when `Cgl` is compiled.

The Cgl website is https://github.com/coin-or/Cgl.

The project managers of Cgl are Robin Lougee (@rlougee) and Francois Margot.

Available cut generators are:

 * Combinatorial cuts:
   * [CglAllDifferent](https://github.com/coin-or/Cgl/wiki/CglAllDifferent)
   * [CglClique](https://github.com/coin-or/Cgl/wiki/CglClique)
   * [CglKnapsackCover](https://github.com/coin-or/Cgl/wiki/CglKnapsackCover)
   * [CglOddHole](https://github.com/coin-or/Cgl/wiki/CglOddHole)
   * CglZeroHalf

 * Flow cover cuts:
   * [CglFlowCover](https://github.com/coin-or/Cgl/wiki/CglFlowCover)

 * Gomory cuts and variants:
   * [CglGomory](https://github.com/coin-or/Cgl/wiki/CglGomory)
   * CglGMI
   * [CglRedSplit](https://github.com/coin-or/Cgl/wiki/CglRedSplit)
   * CglRedSplit2

 * Lift-and-project cuts:
   * [CglLiftAndProject](https://github.com/coin-or/Cgl/wiki/CglLiftAndProject)
   * [CglLandP](https://github.com/coin-or/Cgl/wiki/CglLandP)

 * Mixed integer rounding cuts and variants:
   * [CglMixedIntegerRounding](https://github.com/coin-or/Cgl/wiki/CglMixedIntegerRounding)
   * [CglMixedIntegerRounding2](https://github.com/coin-or/Cgl/wiki/CglMixedIntegerRounding2)
   * [CglTwomir](https://github.com/coin-or/Cgl/wiki/CglTwomir)
   * [CglResidualCapacity](https://github.com/coin-or/Cgl/wiki/CglResidualCapacity)

 * Strengthening:
   * [CglDuplicateRow](https://github.com/coin-or/Cgl/wiki/CglDuplicateRow)
   * [CglPreprocess](https://github.com/coin-or/Cgl/wiki/CglPreprocess)
   * [CglProbing](https://github.com/coin-or/Cgl/wiki/CglProbing)
   * [CglSimpleRounding](https://github.com/coin-or/Cgl/wiki/CglSimpleRounding)


## Getting Started using CoinBrew

To build Cgl from source, obtain the `coinbrew` script from
https://coin-or.github.io/coinbrew/
and run


    /path/to/coinbrew fetch --main-proj=Cgl
    /path/to/coinbrew build --main-proj=Cgl --test
    /path/to/coinbrew install --main-proj=Cgl


The `coinbrew` script will fetch [these](Dependencies) additional projects.

## Getting Started without CoinBrew (Expert users)

 0. Install [these Dependencies](Dependencies)
 1. Obtain the source code, e.g., from https://github.com/coin-or/Cgl
 2. Run `./configure -C` to generate makefiles
 3. Run `make` to build the CoinUtils library
 4. Run `make test` to build and run the CoinUtils unit test program
 5. Run `make install` to install library and header files.

## Project Links

 * [Cgl Wiki](https://github.com/coin-or/Cgl/wiki) with more information on available cut generators
 * [Doxygen-generated html documentation](http://www.coin-or.org/Doxygen/Cgl)
 * [Cgl general mailing list](http://list.coin-or.org/mailman/listinfo/cgl), [Cgl subproject managers mailing list](http://list.coin-or.org/mailman/listinfo/cgl-managers)
 * [Report a bug](https://github.com/coin-or/Cgl/issues/new)

---------

## Information for Subproject Managers

A cut generator in `Cgl` must conform to the following:

 * Its main class `CglCutGeneratorDeriv` is derived from the class `CglCutGenerator`.
 * It has three related classes used for data, parameters and information with respect to the enumeration tree:
   * A class `CglDataDeriv` derived from `CglData`; it should contain pointers on all data used by the generator that might be obtained from an `OsiSolverInterface` object when calling `generateCuts()` with an `OsiSolverInterface` object as parameter. The class `CglDataDeriv` might be `CglData` if the latter is sufficient. An exception is made for generators needing information deemed too expensive to collect from the solver (for example the optimal Simplex tableau); in this case `CglDataDeriv` might still contain a pointer on the `OsiSolverInterface` object, but its use should be limited to obtaining the "expensive" information from the solver.
   * A class `CglParamDeriv` derived from `CglParam`. It should contain parameters of the generator that can be set by the user. The parameters in the class `CglParamDeriv` must be taken into account during the cut generation. The class `CglParamDeriv` might be `CglParam` if the latter is sufficient.
   * A class `CglTreeInfoDeriv` derived from `CglTreeInfo`. The class `CglTreeInfoDeriv` might be `CglTreeInfo` if the latter is sufficient.
 * The class `CglCutGeneratorDeriv` must have 
   * A member of type `CglParamDeriv` used to store the current parameters.
   * A method `getParam()` that returns the object storing the current parameters.
   * A method `generateCuts(const OsiSolverInterface & si, OsiCuts & cs, const CglTreeInfoDeriv info)`
   * A method `generateCuts(const CglDataDeriv &data, OsiCuts & cs, const CglTreeInfoDeriv info)`
 * The data class `CglDataDeriv` must have methods `getMember()` and `setMember()` for each `member` of the class. Data members in `CglData` irrelevant for a generator are completely ignored. If a data member that is used by a generator is not available when `generateCuts(const CglDataDeriv &data, OsiCuts & cs, const CglTreeInfoDeriv info)` is called, the call is aborted, as if no cuts were found. A warning message might be printed.
 * The class `CglParamDeriv` must have methods `getMember()` and `setMember()` for each `member` of the class. All parameters must have default values. Each cut generator with a derived class is free to change the default values for all the members of `CglParamDeriv`, including those from `CglParam`.
 * Once an object of the cut generator class is created, it should be possible to call generateCuts() several times in a row without having to destroy and re-create the object.
 * By default, a successful call to `generateCuts()` should not generate any output. If an error occurs, a message might be printed.


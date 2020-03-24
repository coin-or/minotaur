# Install script for directory: /home/mustafa.vora/minotaur/src/base

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX ".")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so"
         RPATH ":/home/mustafa.vora/minotaur/build/./lib")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/mustafa.vora/minotaur/build/src/base/libminotaur.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so"
         OLD_RPATH "::::::::::::::::::::::::::::::::::::::::"
         NEW_RPATH ":/home/mustafa.vora/minotaur/build/./lib")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libminotaur.so")
    endif()
  endif()
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/minotaur" TYPE FILE FILES
    "/home/mustafa.vora/minotaur/src/base/MinotaurDeconfig.h"
    "/home/mustafa.vora/minotaur/src/base/ActiveNodeStore.h"
    "/home/mustafa.vora/minotaur/src/base/BndProcessor.h"
    "/home/mustafa.vora/minotaur/src/base/Branch.h"
    "/home/mustafa.vora/minotaur/src/base/Brancher.h"
    "/home/mustafa.vora/minotaur/src/base/BranchAndBound.h"
    "/home/mustafa.vora/minotaur/src/base/BrCand.h"
    "/home/mustafa.vora/minotaur/src/base/BrVarCand.h"
    "/home/mustafa.vora/minotaur/src/base/CGraph.h"
    "/home/mustafa.vora/minotaur/src/base/CNode.h"
    "/home/mustafa.vora/minotaur/src/base/ConBoundMod.h"
    "/home/mustafa.vora/minotaur/src/base/Constraint.h"
    "/home/mustafa.vora/minotaur/src/base/CoverCutGenerator.h"
    "/home/mustafa.vora/minotaur/src/base/CutInfo.h"
    "/home/mustafa.vora/minotaur/src/base/CutManager.h"
    "/home/mustafa.vora/minotaur/src/base/CxQuadHandler.h"
    "/home/mustafa.vora/minotaur/src/base/CxUnivarHandler.h"
    "/home/mustafa.vora/minotaur/src/base/Eigen.h"
    "/home/mustafa.vora/minotaur/src/base/Engine.h"
    "/home/mustafa.vora/minotaur/src/base/Environment.h"
    "/home/mustafa.vora/minotaur/src/base/FeasibilityPump.h"
    "/home/mustafa.vora/minotaur/src/base/Exception.h"
    "/home/mustafa.vora/minotaur/src/base/Function.h"
    "/home/mustafa.vora/minotaur/src/base/Handler.h"
    "/home/mustafa.vora/minotaur/src/base/HessianOfLag.h"
    "/home/mustafa.vora/minotaur/src/base/Heuristic.h"
    "/home/mustafa.vora/minotaur/src/base/Iterate.h"
    "/home/mustafa.vora/minotaur/src/base/IntVarHandler.h"
    "/home/mustafa.vora/minotaur/src/base/Jacobian.h"
    "/home/mustafa.vora/minotaur/src/base/KnapsackList.h"
    "/home/mustafa.vora/minotaur/src/base/KnapCovHandler.h"
    "/home/mustafa.vora/minotaur/src/base/LexicoBrancher.h"
    "/home/mustafa.vora/minotaur/src/base/LinearCut.h"
    "/home/mustafa.vora/minotaur/src/base/LinearFunction.h"
    "/home/mustafa.vora/minotaur/src/base/LinearHandler.h"
    "/home/mustafa.vora/minotaur/src/base/LinFeasPump.h"
    "/home/mustafa.vora/minotaur/src/base/Linearizations.h"
    "/home/mustafa.vora/minotaur/src/base/LinBil.h"
    "/home/mustafa.vora/minotaur/src/base/LinConMod.h"
    "/home/mustafa.vora/minotaur/src/base/LinMods.h"
    "/home/mustafa.vora/minotaur/src/base/LGCIGenerator.h"
    "/home/mustafa.vora/minotaur/src/base/Logger.h"
    "/home/mustafa.vora/minotaur/src/base/LPEngine.h"
    "/home/mustafa.vora/minotaur/src/base/LPRelaxation.h"
    "/home/mustafa.vora/minotaur/src/base/MaxFreqBrancher.h"
    "/home/mustafa.vora/minotaur/src/base/MaxVioBrancher.h"
    "/home/mustafa.vora/minotaur/src/base/MILPEngine.h"
    "/home/mustafa.vora/minotaur/src/base/MINLPDiving.h"
    "/home/mustafa.vora/minotaur/src/base/Modification.h"
    "/home/mustafa.vora/minotaur/src/base/MsProcessor.h"
    "/home/mustafa.vora/minotaur/src/base/MultilinearTermsHandler.h"
    "/home/mustafa.vora/minotaur/src/base/NLPEngine.h"
    "/home/mustafa.vora/minotaur/src/base/NLPRelaxation.h"
    "/home/mustafa.vora/minotaur/src/base/NlPresHandler.h"
    "/home/mustafa.vora/minotaur/src/base/NLPMultiStart.h"
    "/home/mustafa.vora/minotaur/src/base/NlWriter.h"
    "/home/mustafa.vora/minotaur/src/base/Node.h"
    "/home/mustafa.vora/minotaur/src/base/NodeHeap.h"
    "/home/mustafa.vora/minotaur/src/base/NodeRelaxer.h"
    "/home/mustafa.vora/minotaur/src/base/NodeIncRelaxer.h"
    "/home/mustafa.vora/minotaur/src/base/NodeProcessor.h"
    "/home/mustafa.vora/minotaur/src/base/NodeStack.h"
    "/home/mustafa.vora/minotaur/src/base/NonlinearFunction.h"
    "/home/mustafa.vora/minotaur/src/base/OAHandler.h"
    "/home/mustafa.vora/minotaur/src/base/Objective.h"
    "/home/mustafa.vora/minotaur/src/base/Operations.h"
    "/home/mustafa.vora/minotaur/src/base/Option.h"
    "/home/mustafa.vora/minotaur/src/base/OpCode.h"
    "/home/mustafa.vora/minotaur/src/base/ParBndProcessor.h"
    "/home/mustafa.vora/minotaur/src/base/ParBranchAndBound.h"
    "/home/mustafa.vora/minotaur/src/base/ParCutMan.h"
    "/home/mustafa.vora/minotaur/src/base/ParMINLPDiving.h"
    "/home/mustafa.vora/minotaur/src/base/ParNodeIncRelaxer.h"
    "/home/mustafa.vora/minotaur/src/base/ParQGBranchAndBound.h"
    "/home/mustafa.vora/minotaur/src/base/ParQGHandler.h"
    "/home/mustafa.vora/minotaur/src/base/ParPCBProcessor.h"
    "/home/mustafa.vora/minotaur/src/base/ParReliabilityBrancher.h"
    "/home/mustafa.vora/minotaur/src/base/ParTreeManager.h"
    "/home/mustafa.vora/minotaur/src/base/PCBProcessor.h"
    "/home/mustafa.vora/minotaur/src/base/PerspCon.h"
    "/home/mustafa.vora/minotaur/src/base/PerspCutHandler.h"
    "/home/mustafa.vora/minotaur/src/base/PolynomialFunction.h"
    "/home/mustafa.vora/minotaur/src/base/PreAuxVars.h"
    "/home/mustafa.vora/minotaur/src/base/PreDelVars.h"
    "/home/mustafa.vora/minotaur/src/base/PreMod.h"
    "/home/mustafa.vora/minotaur/src/base/Presolver.h"
    "/home/mustafa.vora/minotaur/src/base/PreSubstVars.h"
    "/home/mustafa.vora/minotaur/src/base/Problem.h"
    "/home/mustafa.vora/minotaur/src/base/ProblemSize.h"
    "/home/mustafa.vora/minotaur/src/base/ProbStructure.h"
    "/home/mustafa.vora/minotaur/src/base/QPEngine.h"
    "/home/mustafa.vora/minotaur/src/base/QGHandler.h"
    "/home/mustafa.vora/minotaur/src/base/QGHandlerAdvance.h"
    "/home/mustafa.vora/minotaur/src/base/QPDProcessor.h"
    "/home/mustafa.vora/minotaur/src/base/QuadHandler.h"
    "/home/mustafa.vora/minotaur/src/base/QPDRelaxer.h"
    "/home/mustafa.vora/minotaur/src/base/QuadraticFunction.h"
    "/home/mustafa.vora/minotaur/src/base/RandomBrancher.h"
    "/home/mustafa.vora/minotaur/src/base/RCHandler.h"
    "/home/mustafa.vora/minotaur/src/base/Relaxation.h"
    "/home/mustafa.vora/minotaur/src/base/ReliabilityBrancher.h"
    "/home/mustafa.vora/minotaur/src/base/SecantMod.h"
    "/home/mustafa.vora/minotaur/src/base/SimpleCutMan.h"
    "/home/mustafa.vora/minotaur/src/base/SimpleTransformer.h"
    "/home/mustafa.vora/minotaur/src/base/Solution.h"
    "/home/mustafa.vora/minotaur/src/base/SolutionPool.h"
    "/home/mustafa.vora/minotaur/src/base/SOS.h"
    "/home/mustafa.vora/minotaur/src/base/SOS1Handler.h"
    "/home/mustafa.vora/minotaur/src/base/SOS2Handler.h"
    "/home/mustafa.vora/minotaur/src/base/SOSBrCand.h"
    "/home/mustafa.vora/minotaur/src/base/STOAHandler.h"
    "/home/mustafa.vora/minotaur/src/base/Timer.h"
    "/home/mustafa.vora/minotaur/src/base/Transformer.h"
    "/home/mustafa.vora/minotaur/src/base/TransPoly.h"
    "/home/mustafa.vora/minotaur/src/base/TreeManager.h"
    "/home/mustafa.vora/minotaur/src/base/Types.h"
    "/home/mustafa.vora/minotaur/src/base/WarmStart.h"
    "/home/mustafa.vora/minotaur/src/base/Variable.h"
    "/home/mustafa.vora/minotaur/src/base/YEqCGs.h"
    "/home/mustafa.vora/minotaur/src/base/YEqLFs.h"
    "/home/mustafa.vora/minotaur/src/base/YEqMonomial.h"
    "/home/mustafa.vora/minotaur/src/base/YEqUCGs.h"
    "/home/mustafa.vora/minotaur/src/base/YEqVars.h"
    "/home/mustafa.vora/minotaur/src/base/YEqQf.h"
    )
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/minotaur" TYPE FILE FILES "/home/mustafa.vora/minotaur/build/src/base/MinotaurConfig.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/minotaur" TYPE FILE FILES "/home/mustafa.vora/minotaur/build/src/base/MinotaurCFortran.h")
endif()

if("${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/minotaur" TYPE FILE FILES "/home/mustafa.vora/minotaur/build/src/base/Version.h")
endif()


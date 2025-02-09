CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/base/MinotaurConfig.h.cmake"
  "${PROJECT_BINARY_DIR}/src/base/MinotaurConfig.h"
  )

configure_file (
  "${PROJECT_SOURCE_DIR}/src/base/Version.h.cmake"
  "${PROJECT_BINARY_DIR}/src/base/Version.h"
  )

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}/src/base")
include_directories("${PROJECT_SOURCE_DIR}/src/base")
include_directories("${PROJECT_SOURCE_DIR}/src/interfaces")
 
set (MINOTAUR_SOURCES
     base/AnalyticalCenter.cpp
     base/BndProcessor.cpp 
     base/Branch.cpp 
     base/BranchAndBound.cpp 
     base/Brancher.cpp 
     base/BrCand.cpp 
     base/BrVarCand.cpp 
     base/Chol.cpp
     base/CGraph.cpp
     base/CNode.cpp
     base/ConBoundMod.cpp
     base/Constraint.cpp
     base/CoverCutGenerator.cpp 
     base/Cut.cpp
     base/CutInfo.cpp
     base/CutMan1.cpp
     base/CutMan2.cpp
     base/CxQuadHandler.cpp 
     base/CxUnivarHandler.cpp
     base/Eigen.cpp 
     base/Engine.cpp 
     base/Environment.cpp 
     base/FeasibilityPump.cpp 
     base/FixVarsHeur.cpp
     base/Function.cpp 
     base/Handler.cpp
     base/HessianOfLag.cpp 
     base/IntVarHandler.cpp 
     base/Jacobian.cpp
     base/KnapsackList.cpp 
     base/KnapCovHandler.cpp 
     base/LGCIGenerator.cpp 
     base/LPRelaxation.cpp 
     base/LexicoBrancher.cpp 
     base/LinBil.cpp 
     base/LinConMod.cpp 
     base/LinMods.cpp 
     base/LinearCut.cpp 
     base/LinearFunction.cpp 
     base/LinearHandler.cpp
     base/LinFeasPump.cpp 
     base/Linearizations.cpp
     base/Logger.cpp 
     base/MaxFreqBrancher.cpp
     base/MaxInfBrancher.cpp
     base/MaxVioBrancher.cpp
     base/MINLPDiving.cpp
     #base/MultiSolHeur.cpp
     base/MsProcessor.cpp	
     base/MultilinearTermsHandler.cpp
     base/NLPRelaxation.cpp 
     base/NlPresHandler.cpp
     base/NLPMultiStart.cpp
     base/NlWriter.cpp
     base/Node.cpp 
     base/NodeFullRelaxer.cpp
     base/NodeHeap.cpp 
     base/NodeIncRelaxer.cpp 
     base/NodeProcessor.cpp 
     base/NodeStack.cpp 
     base/NonlinearFunction.cpp 
     base/OAHandler.cpp
     base/Objective.cpp 
     base/Operations.cpp 
     base/Option.cpp 
     base/ParBndProcessor.cpp
     base/ParBranchAndBound.cpp
     base/ParCutMan.cpp
     base/ParMINLPDiving.cpp
     base/ParNodeIncRelaxer.cpp
     base/ParQGBranchAndBound.cpp
     base/ParQGHandler.cpp
     base/ParQGHandlerAdvance.cpp
     base/ParPCBProcessor.cpp
     base/ParReliabilityBrancher.cpp
     base/ParTreeManager.cpp
     base/PCBProcessor.cpp 
     base/PerspCon.cpp
     base/PerspCutGenerator.cpp 
     #base/PerspCutHandler.cpp 
     base/PolynomialFunction.cpp 
     base/PreAuxVars.cpp
     base/PreDelVars.cpp
     base/PreSubstVars.cpp
     base/Presolver.cpp 
     base/Problem.cpp
     base/ProbStructure.cpp 
     #base/QGAdvHandler.cpp 
     base/QGHandler.cpp 
     #base/QGHandlerAdvance.cpp 
     #base/QGHandlerPDE.cpp 
     base/QuadHandler.cpp 
     base/kPowHandler.cpp 
     base/QuadraticFunction.cpp
     base/QuadTransformer.cpp 
     base/RandomBrancher.cpp
     base/RCHandler.cpp
     base/Relaxation.cpp 
     base/ReliabilityBrancher.cpp 
     base/Reader.cpp 
     base/SamplingHeur.cpp
     base/SecantMod.cpp 
     base/SimpleCutMan.cpp 
     base/SimpleTransformer.cpp
     base/SimplexQuadCutGen.cpp
     base/Solution.cpp 
     base/SolutionPool.cpp 
     base/SOS.cpp 
     base/SOS1Handler.cpp
     base/SOS2Handler.cpp
     base/SOSBrCand.cpp
     base/SppHeur.cpp
     base/STOAHandler.cpp
     base/HybridBrancher.cpp
     base/Transformer.cpp 
     base/TransPoly.cpp 
     base/TransSep.cpp
     base/TreeManager.cpp 
     base/UnivarQuadHandler.cpp
     base/WeakBrancher.cpp
     base/Types.cpp 
     base/UnambRelBrancher.cpp
     base/VarBoundMod.cpp 
     base/Variable.cpp
     base/YEqBivar.cpp 
     base/YEqCGs.cpp
     base/YEqLFs.cpp
     base/YEqMonomial.cpp
     base/YEqUCGs.cpp
     base/YEqUnivar.cpp
     base/YEqVars.cpp
     base/YEqQfBil.cpp
)

set (MINOTAUR_HEADERS
     base/MinotaurDeconfig.h
     base/ActiveNodeStore.h
     base/AnalyticalCenter.h
     base/BndProcessor.h
     base/Branch.h
     base/Brancher.h
     base/BranchAndBound.h
     base/BrCand.h
     base/BrVarCand.h
     base/CGraph.h
     base/CNode.h
     base/ConBoundMod.h
     base/Constraint.h
     base/CoverCutGenerator.h # Serdar
     base/CutInfo.h
     base/CutManager.h
     base/CxQuadHandler.h 
     base/CxUnivarHandler.h
     base/Eigen.h
     base/Engine.h
     base/Environment.h
     base/FeasibilityPump.h 
     base/FixVarsHeur.h
     base/Exception.h
     base/Function.h
     base/Handler.h
     base/HessianOfLag.h
     base/Heuristic.h
     base/Iterate.h
     base/IntVarHandler.h
     base/Jacobian.h
     base/KnapsackList.h # Serdar
     base/KnapCovHandler.h # Serdar
     base/LexicoBrancher.h
     base/LinearCut.h
     base/LinearFunction.h
     base/LinearHandler.h
     base/LinFeasPump.h 
     base/Linearizations.h
     base/LinBil.h
     base/LinConMod.h
     base/LinMods.h
     base/LGCIGenerator.h # Serdar
     base/Logger.h
     base/LPEngine.h
     base/LPRelaxation.h
     base/MaxFreqBrancher.h
     base/MaxInfBrancher.h
     base/MaxVioBrancher.h
     base/MILPEngine.h
     base/MINLPDiving.h
     #base/MultiSolHeur.h
     base/Modification.h
     base/MsProcessor.h
     base/MultilinearTermsHandler.h
     base/NLPEngine.h
     base/NLPRelaxation.h
     base/NlPresHandler.h
     base/NLPMultiStart.h
     base/NlWriter.h
     base/Node.h
     base/NodeHeap.h
     base/NodeRelaxer.h
     base/NodeIncRelaxer.h
     base/NodeProcessor.h
     base/NodeStack.h
     base/NonlinearFunction.h
     base/OAHandler.h
     base/Objective.h
     base/Operations.h
     base/Option.h
     base/OpCode.h
     base/ParBndProcessor.h
     base/ParBranchAndBound.h
     base/ParCutMan.h
     base/ParMINLPDiving.h
     base/ParNodeIncRelaxer.h
     base/ParQGBranchAndBound.h
     base/ParQGHandler.h
     base/ParQGHandlerAdvance.h
     base/ParPCBProcessor.h
     base/ParReliabilityBrancher.h
     base/ParTreeManager.h
     base/PCBProcessor.h
     base/PerspCon.h
     base/PerspCutGenerator.h 
     #base/PerspCutHandler.h
     base/PolynomialFunction.h
     base/PreAuxVars.h
     base/PreDelVars.h
     base/PreMod.h
     base/Presolver.h
     base/PreSubstVars.h
     base/Problem.h
     base/ProblemSize.h
     base/ProbStructure.h # Serdar
     base/QPEngine.h
     base/QGHandler.h
     base/QGHandlerAdvance.h 
     #base/QGAdvHandler.h 
     #base/QGHandlerPDE.h
     base/QuadHandler.h
     base/kPowHandler.h 
     base/QuadraticFunction.h
     base/QuadTransformer.h
     base/RandomBrancher.h
     base/RCHandler.h
     base/Relaxation.h
     base/ReliabilityBrancher.h
     base/SecantMod.h
     base/SimpleCutMan.h 
     base/SimpleTransformer.h
     base/SimplexQuadCutGen.h
     base/Solution.h
     base/SolutionPool.h
     base/SOS.h
     base/SOS1Handler.h
     base/SOS2Handler.h
     base/SOSBrCand.h
     base/SppHeur.h
     base/STOAHandler.h
     base/HybridBrancher.h
     base/Timer.h
     base/Transformer.h 
     base/TransPoly.h 
     base/TransSep.h
     base/TreeManager.h
     base/SamplingHeur.h
     base/Types.h
     base/UnivarQuadHandler.h
     base/UnambRelBrancher.h
     base/WeakBrancher.h
     base/VarBoundMod.h 
     base/WarmStart.h
     base/Variable.h
     base/YEqBivar.h
     base/YEqCGs.h
     base/YEqLFs.h
     base/YEqMonomial.h
     base/YEqUCGs.h
     base/YEqUnivar.h
     base/YEqVars.h
     base/YEqQfBil.h
     )

install(FILES ${MINOTAUR_HEADERS} DESTINATION include/minotaur)

include_directories("${PROJECT_SOURCE_DIR}/src/interfaces")

set (ENGFAC_SOURCES
  interfaces/EngineFactory.cpp
)
set (ENGFAC_HEADERS
  interfaces/EngineFactory.h
)
if (LINK_CPX)
  add_definitions(-DUSE_CPX -DIL_STD)
  include_directories("${CPX_INC_DIR_F}")
  include_directories("${CPX_INC_DIR_F}/../../concert/include")
  link_directories(${CPX_LIB_DIR_F})

  list(APPEND ENGFAC_SOURCES
              interfaces/CplexMILPEngine.cpp
              interfaces/CplexLPEngine.cpp
  )
  list (APPEND ENGFAC_HEADERS
               interfaces/CplexMILPEngine.h
               interfaces/CplexLPEngine.h
  )
endif()

if (LINK_IPOPT)
  add_definitions(-DUSE_IPOPT)
  include_directories("${IPOPT_INC_DIR_F}")
  link_directories(${IPOPT_LIB_DIR_F})

  list(APPEND ENGFAC_SOURCES 
              interfaces/IpoptDummy.f
              interfaces/IpoptEngine.cpp 
  )
  list (APPEND ENGFAC_HEADERS
               interfaces/IpoptEngine.h 
               interfaces/IpoptEngineTnlp.h
  )
endif()

if (LINK_FILTER_SQP)
  add_definitions(-DUSE_BQPD -DUSE_FILTERSQP)
  list (APPEND ENGFAC_SOURCES 
    interfaces/BqpdEngine.cpp 
    interfaces/BqpdAux.f 
    interfaces/BqpdAux2.f 
    interfaces/FilterSQPEngine.cpp
  )
  list (APPEND ENGFAC_HEADERS 
    interfaces/BqpdEngine.h 
    interfaces/FilterSQPEngine.h 
  )
endif()

if (LINK_UNO)
  #set(CXX_FLAGS "-std=c++17")
  add_definitions(-DUSE_UNO)
  #include_directories("/home/22m1526/Uno/uno")
  link_directories(${UNO_LIB_DIR_F})
  if (UNO_INC_DIR_F)
    include_directories("${UNO_INC_DIR_F}")
  endif (UNO_INC_DIR_F)

  add_definitions(-DCOIN_BIG_INDEX=0) ## avoid warnings
  list (APPEND ENGFAC_SOURCES 
    interfaces/UnoEngine.cpp
    #interfaces/UnoModels.cpp
  )
  list (APPEND ENGFAC_HEADERS 
    interfaces/UnoEngine.h 
    interfaces/UnoModel.h
  )
endif()

if (LINK_OSI)
  add_definitions(-DUSE_OSILP)
  link_directories(${OSI_LIB_DIR_F})
  if (OSI_INC_DIR_F)
    include_directories("${OSI_INC_DIR_F}")
  endif (OSI_INC_DIR_F)
  if (CLP_INC_DIR_F)
    include_directories("${CLP_INC_DIR_F}")
  endif (CLP_INC_DIR_F)

  add_definitions(-DCOIN_BIG_INDEX=0) ## avoid warnings
  list (APPEND ENGFAC_SOURCES 
    interfaces/OsiLPEngine.cpp 
  )
  list (APPEND ENGFAC_HEADERS 
    interfaces/OsiLPEngine.h 
  )
endif()



set (IFACE_SOURCES)
set (IFACE_HEADERS)
if (LINK_ASL)
  add_definitions(-DUSE_MINOTAUR_AMPL_INTERFACE)
  include_directories("${ASL_INC_DIR_F}")
  list (APPEND IFACE_SOURCES 
    interfaces/AMPLHessian.cpp
    interfaces/AMPLInterface.cpp
    interfaces/AMPLJacobian.cpp
    interfaces/AMPLNonlinearFunction.cpp
  )
  set (IFACE_HEADERS
    interfaces/AMPLHessian.h
    interfaces/AMPLInterface.h
    interfaces/AMPLJacobian.h
    interfaces/AMPLNonlinearFunction.h
  )
endif()

set (GFORTRAN_COMPILER)
if (Fortran_COMPILER_NAME STREQUAL "gfortran")
  if (LINK_IPOPT OR LINK_FILTER_SQP)
    set (GFORTRAN_COMPILER gfortran)
  endif()
endif()

# Add all nonempty libs
set (ALL_EXEC_LIBS "minotaur")

if (IPOPT_LIBS)
  list(APPEND ALL_EXEC_LIBS "${IPOPT_LIBS}")
endif()
if (CPX_LIBS)
  list(APPEND ALL_EXEC_LIBS "${CPX_LIBS}")
endif()
if (OSI_LIBS)
  list(APPEND ALL_EXEC_LIBS "${OSI_LIBS}")
endif()
if (ASL_LIBS)
  list(APPEND ALL_EXEC_LIBS "${ASL_LIBS}")
endif()
if (FILTER_SQP_LIBS)
  list(APPEND ALL_EXEC_LIBS ${FILTER_SQP_LIBS})
endif()
if (UNO_LIBS)
  list(APPEND ALL_EXEC_LIBS "${UNO_LIBS}")
endif()

list(APPEND ALL_EXEC_LIBS "lapack" "blas")

if (GFORTRAN_COMPILER)
  list(APPEND ALL_EXEC_LIBS "${GFORTRAN_COMPILER}")
endif()
if (MNTR_EXTRA_LIBS)
  list(APPEND ALL_EXEC_LIBS "${MNTR_EXTRA_LIBS}")
endif()

message(STATUS ${MSG_HEAD} "ALL_EXEC_LIBS =  ${ALL_EXEC_LIBS}")
list(REVERSE ALL_EXEC_LIBS)
list(REMOVE_DUPLICATES ALL_EXEC_LIBS)
list(REVERSE ALL_EXEC_LIBS)

message(STATUS ${MSG_HEAD} "ALL_EXEC_LIBS after removing duplicates =  ${ALL_EXEC_LIBS}")

if (IFACE_SOURCES)
  set (SOLVER_SOURCES
    solvers/Solver.cpp
    solvers/Bnb.cpp 
    solvers/Glob.cpp 
    solvers/MultiStart.cpp 
    solvers/SolCheck.cpp 
    solvers/QG.cpp 
    solvers/QGPar.cpp 
    solvers/BnbPar.cpp 
  )
  set (SOLVER_HEADERS
    solvers/Solver.h
    solvers/Bnb.h 
    solvers/Glob.h 
    solvers/MultiStart.h 
    solvers/SolCheck.h 
    solvers/QG.h 
    solvers/QGPar.h 
    solvers/BnbPar.h 
  )
endif()

add_library(minotaur ${MINOTAUR_SOURCES} ${ENGFAC_SOURCES} ${IFACE_SOURCES} ${SOLVER_SOURCES})
if (BUILD_SHARED_LIBS)
  install(TARGETS minotaur 
          LIBRARY 
          DESTINATION lib)
else (BUILD_SHARED_LIBS)
  install(TARGETS minotaur 
          ARCHIVE 
          DESTINATION lib)
endif (BUILD_SHARED_LIBS)

if (IFACE_SOURCES)
  add_executable(mbnb solvers/BnbMain.cpp)
  target_link_libraries(mbnb ${ALL_EXEC_LIBS})
  install(TARGETS mbnb RUNTIME DESTINATION bin)
  set_target_properties(mbnb PROPERTIES INSTALL_RPATH "${MNTR_INSTALL_RPATH}")
  
  add_executable(mqg solvers/QGMain.cpp)
  target_link_libraries(mqg ${ALL_EXEC_LIBS})
  install(TARGETS mqg RUNTIME DESTINATION bin)
  set_target_properties(mqg PROPERTIES INSTALL_RPATH "${MNTR_INSTALL_RPATH}")
  
  add_executable(mqgpar solvers/QGParMain.cpp)
  target_link_libraries(mqgpar ${ALL_EXEC_LIBS})
  install(TARGETS mqgpar RUNTIME DESTINATION bin)
  set_target_properties(mqgpar PROPERTIES INSTALL_RPATH "${MNTR_INSTALL_RPATH}")
   
  add_executable(mbnbpar solvers/BnbParMain.cpp)
  target_link_libraries(mbnbpar ${ALL_EXEC_LIBS})
  install(TARGETS mbnbpar RUNTIME DESTINATION bin)
  set_target_properties(mbnbpar PROPERTIES INSTALL_RPATH "${MNTR_INSTALL_RPATH}")

  #add_executable(moa solvers/OA.cpp)
  #target_link_libraries(moa ${ALL_EXEC_LIBS})
  #install(TARGETS moa RUNTIME DESTINATION bin)
  #set_target_properties(moa PROPERTIES INSTALL_RPATH "${MNTR_INSTALL_RPATH}")

  #add_executable(mlstoa solvers/LSTOA.cpp)
  #target_link_libraries(mlstoa ${ALL_EXEC_LIBS})
  #install(TARGETS mlstoa RUNTIME DESTINATION bin)
  #set_target_properties(mlstoa PROPERTIES INSTALL_RPATH "${MNTR_INSTALL_RPATH}")

  add_executable(mglob solvers/GlobMain.cpp)
  target_link_libraries(mglob ${ALL_EXEC_LIBS})
  install(TARGETS mglob RUNTIME DESTINATION bin)
  set_target_properties(mglob PROPERTIES INSTALL_RPATH "${MNTR_INSTALL_RPATH}")

  add_executable(mmultistart solvers/MultiStartMain.cpp)
  target_link_libraries(mmultistart ${ALL_EXEC_LIBS})
  install(TARGETS mmultistart RUNTIME DESTINATION bin)
  set_target_properties(mmultistart PROPERTIES INSTALL_RPATH "${MNTR_INSTALL_RPATH}")
  
  add_executable(mcheck solvers/SolCheckMain.cpp)
  target_link_libraries(mcheck ${ALL_EXEC_LIBS})
  install(TARGETS mcheck RUNTIME DESTINATION bin)
  set_target_properties(mcheck PROPERTIES INSTALL_RPATH "${MNTR_INSTALL_RPATH}")
  
endif()

install(FILES ${ENGFAC_HEADERS} DESTINATION include/minotaur)
install(FILES ${IFACE_HEADERS} DESTINATION include/minotaur)
if (IFACE_SOURCES)
  install(FILES ${SOLVER_HEADERS} DESTINATION include/minotaur)
endif()
install(FILES ${PROJECT_BINARY_DIR}/src/base/MinotaurConfig.h DESTINATION include/minotaur)
install(FILES ${PROJECT_BINARY_DIR}/src/base/MinotaurCFortran.h DESTINATION include/minotaur)
install(FILES ${PROJECT_BINARY_DIR}/src/base/Version.h DESTINATION include/minotaur)

######################################################################################################


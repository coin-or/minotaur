//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2011 The MINOTAUR Team
// 

/*! \brief Algorithm for solving (nonconvex) quadratic programs
 *
 * \author Jeff Linderoth, MINOTAUR Team
 */

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include <MinotaurConfig.h>
#include <AMPLHessian.h>
#include <AMPLJacobian.h>
#include <Environment.h>
#include <Handler.h>
#include <Option.h>
//#include <QGHandler.h>
#include <Problem.h>
#include <Engine.h>
#include <QPEngine.h>
#include <LPEngine.h>
#include <NLPEngine.h>
// Jim, we don't need specific engines here. Use Enginefactory to get them.
//#include <FilterSQPEngine.h> 
//#include <OsiLPEngine.h>
#include <NodeRelaxer.h>
#include <NodeIncRelaxer.h>
#include <PCBProcessor.h>
#include <MaxVioBrancher.h>
#include "quasiQGHandler.h"
#include <ReliabilityBrancher.h>
#include <AMPLInterface.h>
#include <BranchAndBound.h>
#include <Presolver.h>
#include <Timer.h>
#include <LexicoBrancher.h>
#include <Logger.h>
#include <LinearHandler.h>
#include <IntVarHandler.h>
#include <Solution.h>
#include <TreeManager.h>
#include <EngineFactory.h>
#include <CxQuadHandler.h>
#include <Objective.h>
//#include <CutMan2.h>
#include <CutManager.h>

using namespace Minotaur;

struct qgStat {
  UInt proc_nodes;
  UInt total_size;
  Double total_time;
  Double ObjVal;
  Double LowerB;
  Size_t nlpS;
  Size_t nlpP;
  Size_t nlpI;
  Size_t nlpF;
  Size_t looseCut;
  Bool QC;
  Bool partialFix;  
  Bool accpm;
  Size_t nlpP1;
  Size_t infCuts;
  Size_t nlpPC;
  Size_t p2Cuts;
  Size_t p1Cuts;
  Size_t CoCuts;
  Size_t adCuts;
  Size_t cuts;
  Size_t ecpCalls;
};

EnginePtr getNLPEngine(EnvPtr env, ProblemPtr p);
EnginePtr getACEngine(EnvPtr env);
EnginePtr getNLPEngine(EnvPtr env);
PresolverPtr createPres(EnvPtr env, ProblemPtr p, Size_t ndefs);
void writeStats(BrancherPtr brancher_, LPEnginePtr lpE_);
void writeStats(std::string fname, UInt proc_nodes, UInt total_size, 
		Double total_time, Double ObjVal , Double LowerB, 
		UInt quasi,Size_t nlpS, Size_t nlpP, Size_t nlpI, Size_t nlpF);

void writeStats(std::string fname,std::string methodname, qgStat *myStat, LPEnginePtr lpEng, 
		Double RelTr, Int PoolTr, Double mngrTime, Int RelSize, Int PoolSize, 
		Int RelToPool, Int PoolToRel, Double RelAve, Double PoolAve, 
		quasiQGHandlerPtr qghand);

//void writeStats(std::string fname, qgStat *myStat, LPEnginePtr lpEng, Int RelTr, 
//		Int PoolTr, Double mngrTime, Int RelSize, Int PoolSize, 
//		Int RelToPool, Int PoolToRel, Double RelAve, Double PoolAve, quasiQGHandlerPtr qghand);


void writeStats(std::string fname, qgStat *myStat, OptionDBPtr options, LPEnginePtr lpEngine , quasiQGHandlerPtr qgPtr, Bool ecp);

void show_help();
void writeSolutionToFile(const Double * x, UInt n);


void show_help()
{
  std::cout << "Usage:" << std::endl
            << "To show version: qg -v (or --show_version yes) " << std::endl
            << "To show all options: qg -= (or --show_options yes)" 
            << std::endl
            << "To solve an instance: qg --option1 [value] "
            << "--option2 [value] ... " << " .nl-file" << std::endl;
}

int main(int argc, char* argv[])
{
  EnvPtr env = (EnvPtr) new Environment();
  Timer *timer    = env->getNewTimer();
  OptionDBPtr options;

  MINOTAUR_AMPL::AMPLInterfacePtr iface = MINOTAUR_AMPL::AMPLInterfacePtr();  
  MINOTAUR_AMPL::AMPLInterfacePtr proj_iface = MINOTAUR_AMPL::AMPLInterfacePtr();  
  MINOTAUR_AMPL::AMPLInterfacePtr l1proj_iface = MINOTAUR_AMPL::AMPLInterfacePtr(); 
  ProblemPtr inst;
  SolutionPtr sol, sol2;

  UInt numvars;
 
  // jacobian is read from AMPL interface and passed on to branch-and-bound
  JacobianPtr jPtr;
  // hessian is read from AMPL interface and passed on to branch-and-bound
  MINOTAUR_AMPL::AMPLHessianPtr hPtr;

  // the branch-and-bound
  BranchAndBoundPtr bab;
  PresolverPtr pres;
  EngineFactory *efac;
  const std::string me("qg: ");

  Bool prune = false;
  BrancherPtr br = BrancherPtr(); // NULL
  PCBProcessorPtr nproc;

  NodeIncRelaxerPtr nr;
  CutMan2Ptr ctMngr;
  //CutManagerPtr ctMngr;

  //handlers
  HandlerVector handlers;
  quasiQGHandlerPtr qghand;
  LinearHandlerPtr l_hand;
  IntVarHandlerPtr v_hand;

  //engines
  EnginePtr nlp_e;
  EnginePtr proj_nlp_e;
  EnginePtr l1proj_nlp_e;
  EnginePtr AC_nlp_e;
  EnginePtr pnt_e;

  LPEnginePtr lin_e;   // lp engine 
  LoggerPtr logger_ = (LoggerPtr) new Logger(LogInfo);
  VarVector *orig_v=0;

  // start timing.
  timer->start();

  iface = (MINOTAUR_AMPL::AMPLInterfacePtr) 
    new MINOTAUR_AMPL::AMPLInterface(env, "qg");

  // parse options
  env->readOptions(argc-1, argv);


  Bool ECP_CUT = false;
  //Double beta_ecp;
  //Double tol_ecp;
  std::string myoption;
  //std::string mymethod;
  myoption = argv[argc-1];
  if (myoption == "e"){
    ECP_CUT = true;
  //  pnt_e = getNLPEngine(env);
  }
  else if (myoption == "n")
    ECP_CUT = false;
  //mymethod = argv[argc-1];
  //myoption = argv[argc-3];
  //tol_ecp = atof(myoption.c_str());
  //myoption = argv[argc-2];
  //beta_ecp = atof(myoption.c_str());
  

/*
  std::cout << "argc = " << argc << "\n";
  if (argv[argc-1] == "e"){
  //if (argv[10] == "1"){
	std::cout << "eeeeeeeeeeeeeeee\n";
	ECP_CUT = true;
  } else if (argv[argc-1] == "" || argv[argc-1] == "n"){
  //} else if (argv[10] == "" || argv[10] == "0"){
	std::cout << "nnnnnnnnnnnnnnnn\n";
	ECP_CUT = false;
  }else std::cout << "no option found\n";
*/
/*
  for (Int i = 0; i < argc; ++i)
  {
    std::cout << "option[" << i << "]=" << argv[i] << "\n";
  }
*/



  //if (options->findBool("ECP")->getValue()==true)
  //else
  //  ECP_CUT = false;

/*
  std::cout << "argc = " << argc << "\n";
  if (argv[argc-1] == "e"){
  //if (argv[10] == "1"){
	std::cout << "eeeeeeeeeeeeeeee\n";
	ECP_CUT = true;
  } else if (argv[argc-1] == "" || argv[argc-1] == "n"){
  //} else if (argv[10] == "" || argv[10] == "0"){
	std::cout << "nnnnnnnnnnnnnnnn\n";
	ECP_CUT = false;
  }else std::cout << "no option found\n";
*/
/*
  for (Int i = 0; i < argc; ++i)
  {
    std::cout << "option[" << i << "]=" << argv[i] << "\n";
  }
*/
  options = env->getOptions();
  options->findString("interface_type")->setValue("AMPL");
  options->findBool("modify_rel_only")->setValue(true);

  qgStat *myStat;
  myStat = new qgStat();
  myStat->proc_nodes = 0;
  myStat->total_size = 0;
  myStat->total_time = 0.0;
  myStat->ObjVal = INFINITY;
  myStat->LowerB = -INFINITY;
  myStat->nlpS = 0;
  myStat->nlpP = 0;
  myStat->nlpI = 0;
  myStat->nlpF = 0;
  myStat->looseCut = 0;
  myStat->adCuts = 0;
  myStat->ecpCalls = 0;
  if(options->findBool("convexity_type")->getValue()==true)
    myStat->QC=true;
  else
    myStat->QC=false;
  myStat->partialFix = options->findBool("partial_bb")->getValue();
  myStat->accpm = options->findBool("qg_accpm")->getValue();

  // check if user needs help.
  if (options->findBool("show_options")->getValue() ||
      options->findFlag("=")->getValue()) {
    options->write(std::cout);
    goto CLEANUP;
  }

//    goto CLEANUP;
//  }

  if (options->findBool("show_version")->getValue() ||
      options->findFlag("v")->getValue()) {
    std::cout << me << "Minotaur version " << env->getVersion() << std::endl;
    goto CLEANUP;
  }

  if (options->findString("problem_file")->getValue()=="") {
    show_help();
    goto CLEANUP;
  }

  std::cout << me << "Minotaur version " << env->getVersion() << std::endl;
  // load the problem.
  inst = iface->readInstance(options->findString("problem_file")->getValue());
  std::cout << me << "time used in reading instance = " << std::fixed 
    << std::setprecision(2) << timer->query() << std::endl;

  inst->calculateSize(true);
  numvars = inst->getNumVars();
  // display the problem
  if (options->findBool("display_problem")->getValue()==true) {
    inst->write(std::cout);
  }

  if (options->findBool("display_size")->getValue()==true) {
    inst->calculateSize();
    inst->writeSize(std::cout);
  }

  // Initialize engines
  nlp_e = getNLPEngine(env, inst);                                //Engine for Original problem

  if (options->findBool("qg_accpm")->getValue()==true) {
  	 AC_nlp_e = getACEngine(env);                             //Engine for Analytic Center Problem
  }

  efac = new EngineFactory(env);
  lin_e = efac->getLPEngine();   // lp engine 
  delete efac;

  // create the jacobian

  if (false==env->getOptions()->findBool("use_native_cgraph")->getValue()){

    if (inst->isQP() || inst->isQuadratic()) {
//      std::cout << "setNativeDer\n";
      inst->setNativeDer();
      //inst->setQuadraticHessian();
    }
    else{
//        std::cout << "set AMPL Der\n";
    	jPtr = (MINOTAUR_AMPL::AMPLJacobianPtr) 
     				 new MINOTAUR_AMPL::AMPLJacobian(iface);
    	inst->setJacobian(jPtr);

    	// create the hessian
    	hPtr = (MINOTAUR_AMPL::AMPLHessianPtr) new MINOTAUR_AMPL::AMPLHessian(iface);
    	inst->setHessian(hPtr);
	 }
  } 
  else 
  {
    inst->setNativeDer();
  }

//  inst->setNativeDer();
/*
  std::cout << "test nlp solve\n";
  nlp_e->load(inst);
  nlp_e->solve();
  std::cout << "nlp status = " << nlp_e->getStatusString() << "\n";
  nlp_e->clear();
*/
  // Initialize the handlers
  l_hand = (LinearHandlerPtr) new LinearHandler(env, inst);
  handlers.push_back(l_hand);
  assert(l_hand);

  v_hand = (IntVarHandlerPtr) new IntVarHandler(env, inst);
  handlers.push_back(v_hand);
  assert(v_hand);

//  std::cout << "hessian number of nonzero 1= " << hPtr->getNumNz() << std::endl;
//  if (options->findBool("partial_bb")->getValue()==true){
//    qghand = (quasiQGHandlerPtr) new quasiQGHandler(env, inst, nlp_e, AC_nlp_e);
//    assert(qghand);
//    handlers.push_back(qghand);
//  } else 
  if (options->findBool("convexity_type")->getValue()==true){
    ProblemPtr proj_inst;
    ProblemPtr l1proj_inst;

    proj_iface = (MINOTAUR_AMPL::AMPLInterfacePtr)
      	new MINOTAUR_AMPL::AMPLInterface(env, "qg");
    l1proj_iface = (MINOTAUR_AMPL::AMPLInterfacePtr)
      	new MINOTAUR_AMPL::AMPLInterface(env, "qg");

//    proj_inst = proj_iface->readInstance(options->findString("problem_file")
//                                         ->getValue());
//    l1proj_inst = l1proj_iface->readInstance(options->findString("problem_file")
//                                         ->getValue());

    options->findBool("use_native_cgraph")->setValue(true);
    proj_inst = proj_iface->readInstance(options->findString("problem_file")
                                          ->getValue());
    l1proj_inst = l1proj_iface->readInstance(options->findString("problem_file")
                                         ->getValue());

    proj_inst->calculateSize();
    l1proj_inst->calculateSize();

    //if (inst->isQP() || inst->isQuadratic())
    //  proj_inst->setQuadraticJacobian();
    //else{
    //  proj_inst->setJacobian(jPtr);
    //}

    proj_nlp_e=getNLPEngine(env, proj_inst);
    proj_inst->calculateSize();

    l1proj_nlp_e=getNLPEngine(env, l1proj_inst);
    l1proj_inst->calculateSize();
//    qghand = (quasiQGHandlerPtr) new quasiQGHandler(env, inst, proj_inst, nlp_e, 1);
    qghand = (quasiQGHandlerPtr) new quasiQGHandler(env, inst, proj_inst, l1proj_inst, nlp_e, proj_nlp_e, l1proj_nlp_e, AC_nlp_e);


    assert(qghand);
    handlers.push_back(qghand);
  } else if (options->findBool("convexity_type")->getValue()==false) {
    qghand = (quasiQGHandlerPtr) new quasiQGHandler(env, inst, nlp_e, AC_nlp_e ); 
    assert(qghand);
    handlers.push_back(qghand);
  }
  qghand->setECP(ECP_CUT);
  //qghand->setBetaECP(beta_ecp);
  //qghand->setTolECP(tol_ecp);
  //if (ECP_CUT);
  //  qghand->setPointProbEngine(pnt_e);
  
//  std::cout << "set initial point" << std::endl;
	// set initial point

  inst->setInitialPoint(iface->getInitialPoint(), 
      inst->getNumVars()-iface->getNumDefs());

//  std::cout << "get presolver" << std::endl;
  // get presolver.
  orig_v = new VarVector(inst->varsBegin(), inst->varsEnd());
  pres = createPres(env, inst, iface->getNumDefs());
  pres->standardize(); 
  if (env->getOptions()->findBool("presolve")->getValue() == true) {
    std::cout << me << "Presolving ... " << std::endl;
    pres->solve();
    std::cout << me << "Finished presolving." << std::endl;
  }
  inst->calculateSize();

  
  // report name
  logger_->msgStream(LogInfo) << me << "handlers used:" << std::endl;
  
  for (HandlerIterator h = handlers.begin(); h != handlers.end(); ++h) {
    logger_->msgStream(LogInfo) << me << (*h)->getName() << std::endl;
  }

  // Stuff needed in branch and bound
  nr = (NodeIncRelaxerPtr) new NodeIncRelaxer(env, handlers);

  // find the type of relaxation.
  if (!prune) {
    nr->setEngine(lin_e);
    nproc = (PCBProcessorPtr) new PCBProcessor(env, lin_e, handlers);

    if (env->getOptions()->findString("brancher")->getValue() == "rel") {
      ReliabilityBrancherPtr rel_br = 
        (ReliabilityBrancherPtr) new ReliabilityBrancher(env, handlers);
      rel_br->setEngine(lin_e);
      nproc->setBrancher(rel_br);
      br = rel_br;
    } else if (env->getOptions()->findString("brancher")->getValue()
      == "maxvio") {
      MaxVioBrancherPtr mbr = (MaxVioBrancherPtr) 
        new MaxVioBrancher(env, handlers);
      nproc->setBrancher(mbr);
      br = mbr;
    } else if (env->getOptions()->findString("brancher")->getValue()
      == "lex") {
      LexicoBrancherPtr lbr = (LexicoBrancherPtr) 
        new LexicoBrancher(env, handlers);
      br = lbr;
    }
    nproc->setBrancher(br);
    std::cout << me << "brancher used = " << br->getName() << std::endl;
    bab = (BranchAndBoundPtr) new BranchAndBound(env, inst);
    bab->setNodeRelaxer(nr);
    bab->setNodeProcessor(nproc);
    bab->shouldCreateRoot(true);
    //ctMngr = (CutMan2Ptr) new CutMan2(env,inst);
    if (ctMngr){
      bab->addCutManager(ctMngr);
      qghand->setCutManager(ctMngr);
    }
  }
  else {
    std::cout << "Problem declared infeasible at initial relaxation step." <<
      std::endl;
    goto CLEANUP;
  }


  // solve
  if (options->findBool("solve")->getValue()==true) {
    // start solving
    bab->solve();

    std::cout << "status of branch-and-bound: " 
      << getSolveStatusString(bab->getStatus()) << std::endl;
    // write solution
    sol = bab->getSolution(); // presolved solution needs translation.
    if (sol) {
      sol2 = pres->getPostSol(sol);
      sol = sol2;
      assert (sol);
      if (options->findFlag("AMPL")->getValue()) {
        iface->writeSolution(sol,bab->getStatus());
      } else {
//        sol->writePrimal(std::cout,orig_v);
      }
    }
    std::cout << "nodes created in branch-and-bound = " << 
      bab->getTreeManager()->getSize() << std::endl;
    // commented out by ashu
    //std::cout << "nodes processed in branch-and-bound 1= " <<
    //  bab->getTreeManager()->getActiveNodes() << std::endl;
    std::cout << std::fixed << std::setprecision(4) << 
      "best bound estimate for remaining nodes = " <<  bab->getLb() 
      << std::endl;
    std::cout << std::fixed << std::setprecision(4) << 
      "best solution value = " <<  bab->getUb() << std::endl;
    writeStats(br,lin_e);
  }
  std::cout << "time used = " << std::fixed << std::setprecision(2) 
    << timer->query() << std::endl;

  std::cout << "number of processed nodes is = " << bab->numProcNodes() << std::endl;

  std::cout << "problem name is " << options->findString("problem_file")->getValue() << std::endl;
//std::string fname, UInt proc_nodes, UInt total_size, Double total_time, UInt quasi)
///void writeStats(std::string fname, UInt proc_nodes, UInt total_size, Double total_time, UInt quasi)
///void writeStats(std::string fname, UInt proc_nodes, UInt total_size, Double total_time, Double ObjVal, UInt quasi)

  myStat->proc_nodes = bab->numProcNodes();
  myStat->total_size = bab->getTreeManager()->getSize();
  myStat->total_time = bab->totalTime();
  myStat->ObjVal = bab->getUb();
  myStat->LowerB = bab->getLb();
  myStat->nlpS = qghand->QG_nlpS();
  myStat->nlpP = qghand->QG_nlpP();
  myStat->nlpI = qghand->QG_nlpI();
  myStat->nlpF = qghand->QG_nlpF();
  myStat->looseCut = qghand->QG_looseCuts();
  myStat->nlpP1 = qghand->QG_nlpP1();
  myStat->infCuts = qghand->QG_infCuts();
  myStat->nlpPC = qghand->QG_nlpPC();
  myStat->p2Cuts = qghand->QG_p2Cuts();
  myStat->p1Cuts = qghand->QG_p1Cuts();
  myStat->CoCuts = qghand->QG_CoCuts();
  myStat->adCuts = qghand->QG_adCuts();
  myStat->cuts = qghand->QG_Cuts();
  myStat->ecpCalls = qghand->QG_ecpCalls();
  std::cout << "my ecp calls = " << qghand->QG_ecpCalls() << "\n";
  //CutMan2::ctMngrInfo ctmngrInfo;// = ctMngr->getInfo();
  //if (ctMngr){
  //  ctmngrInfo = ctMngr->getInfo();
  //}
/*
  if (options->findBool("convexity_type")->getValue()==true)
     writeStats(options->findString("problem_file")->getValue(), bab->numProcNodes() , bab->getTreeManager()->getSize() ,  bab->totalTime(),bab->getUb(), 
                bab->getLb(), 1, qghand->QG_nlpS(), qghand->QG_nlpP(), qghand->QG_nlpI(), qghand->QG_nlpF() );
  else {
    writeStats(options->findString("problem_file")->getValue(), bab->numProcNodes() , bab->getTreeManager()->getSize() ,bab->totalTime(),bab->getUb(), 
               bab->getLb(), 0, qghand->QG_nlpS(), 0, qghand->QG_nlpI(), qghand->QG_nlpF());
//    writeSolutionToFile( bab->getSolution()->getPrimal() , numvars);
  }
*/
  
  writeStats(options->findString("problem_file")->getValue(), myStat, options, lin_e,qghand,ECP_CUT);  
  //if (ctMngr){
  //writeStats(options->findString("problem_file")->getValue(), myStat, lin_e, 
  //		ctmngrInfo.RelTr, ctmngrInfo.PoolTr, ctmngrInfo.t, ctmngrInfo.RelSize,
  //		ctmngrInfo.PoolSize, ctmngrInfo.RelToPool,ctmngrInfo.PoolToRel, 
  //		ctmngrInfo.RelAve,ctmngrInfo.PoolAve, qghand);
/*
  if (env->getOptions()->findBool("ecp")->getValue()==true){
    writeStats(options->findString("problem_file")->getValue(), myStat, lin_e, 
		.001,1,ctmngrInfo.t, ctmngrInfo.RelSize,
		ctmngrInfo.PoolSize, ctmngrInfo.RelToPool,ctmngrInfo.PoolToRel, 
		ctmngrInfo.RelAve,ctmngrInfo.PoolAve,ctmngrInfo.PrntActCnt,qghand);
  } else {
    writeStats(options->findString("problem_file")->getValue(), myStat, lin_e, 
		0,0,ctmngrInfo.t, ctmngrInfo.RelSize,
		ctmngrInfo.PoolSize, ctmngrInfo.RelToPool,ctmngrInfo.PoolToRel, 
		ctmngrInfo.RelAve,ctmngrInfo.PoolAve,ctmngrInfo.PrntActCnt,qghand);
}

*/
/*
    writeStats(options->findString("problem_file")->getValue(), myStat, lin_e, 
		ctmngrInfo.RelTr, ctmngrInfo.PoolTr, ctmngrInfo.t, ctmngrInfo.RelSize,
		ctmngrInfo.PoolSize, ctmngrInfo.RelToPool,ctmngrInfo.PoolToRel, 
		ctmngrInfo.RelAve,ctmngrInfo.PoolAve,ctmngrInfo.PrntActCnt,qghand);
*/

  //}
  //else{

/*
    // Having ECP cuts

    if (ECP_CUT){
      writeStats(options->findString("problem_file")->getValue(), mymethod, myStat, lin_e, 
		tol_ecp,beta_ecp,
		//0.001, env->getOptions()->findDouble("beta")->getValue()
		0.0, 0, 0, 0, 0, 0.0, 0.0,qghand);
    } else {
      writeStats(options->findString("problem_file")->getValue(), mymethod, myStat, lin_e, 
		0,0, 0.0, 0, 0, 0, 0, 0.0, 0.0,qghand);
    }
*/

  //}
/*
		ctmngrInfo.RelTr, ctmngrInfo.PoolTr, ctmngrInfo.t, ctmngrInfo.RelSize,
		ctmngrInfo.PoolSize, ctmngrInfo.RelToPool,ctmngrInfo.PoolToRel, 
		ctmngrInfo.RelAve,ctmngrInfo.PoolAve);
*/


CLEANUP:
  if (iface) {
    delete iface;
  }
  if (orig_v) {
    delete orig_v;
  }
  if (proj_iface && options->findBool("convexity_type")->getValue()==true) {
    delete proj_iface;
  }
  delete timer;
  return 0;
}

EnginePtr getACEngine(EnvPtr env)
{
  EngineFactory *efac = new EngineFactory(env);
  EnginePtr e = EnginePtr();

  // AC Engine should always be an NLP engine (probably should always be IPOPT)
  e = efac->getNLPEngine();

  assert (e || (!"No engine available for this problem."));
  delete efac;
  return e;

  return e;
}

EnginePtr getNLPEngine(EnvPtr env, ProblemPtr p)
{
  EngineFactory *efac = new EngineFactory(env);
  EnginePtr e = EnginePtr(); // NULL
  Bool cont=false;

  p->calculateSize();
  if (p->isLinear()) {
    e = efac->getLPEngine();
    if (e) {
      delete efac;
      return e;
    } else {
      cont = true;
    }
  }

  if (true==cont || p->isQP()) {
    e = efac->getQPEngine();
    if (e) {
      delete efac;
      return e;
    } else {
      cont = true;
    }
  }

  e = efac->getNLPEngine();

  assert (e || (!"No engine available for this problem."));
  delete efac;
  return e;
}

EnginePtr getNLPEngine(EnvPtr env)
{
  EngineFactory *efac = new EngineFactory(env);
  EnginePtr e = EnginePtr();
  e = efac->getQPEngine();
  return e;
}
PresolverPtr createPres(EnvPtr env, ProblemPtr p, Size_t ndefs)
{
  // create handlers for presolve
  HandlerVector handlers;
  PresolverPtr pres = PresolverPtr(); // NULL
  p->calculateSize();
  if (env->getOptions()->findBool("presolve")->getValue() == true) {
    LinearHandlerPtr lhandler = (LinearHandlerPtr) new LinearHandler(env, p);
    handlers.push_back(lhandler);
    if (p->isQP() || p->isQuadratic() || p->isLinear()) {
      lhandler->setPreOptPurgeVars(true);
      lhandler->setPreOptPurgeCons(true);
    } else {
      lhandler->setPreOptPurgeVars(false);
      lhandler->setPreOptPurgeCons(false);
    }
    if (ndefs>0) {
      lhandler->setPreOptDualFix(false);
    } else {
      lhandler->setPreOptDualFix(true);
    }

    if (p->isQP() || p->isQuadratic()) {
      CxQuadHandlerPtr cx_quad_hand = (CxQuadHandlerPtr) 
        new CxQuadHandler(env, p);
      handlers.push_back(cx_quad_hand);
    }

    // write the names.
    std::cout << "handlers used in presolve:" << std::endl;
    for (HandlerIterator h = handlers.begin(); h != handlers.end(); 
        ++h) {
      std::cout<<(*h)->getName()<<std::endl;
    }
  }

  pres = (PresolverPtr) new Presolver(p, env, handlers);
  return pres;
}



void writeStats(BrancherPtr brancher_, LPEnginePtr lpE_ )
{
  std::cout << "write Stats" << std::endl;
  if (brancher_) {
    brancher_->writeStats();
  }
  if (lpE_) {
    lpE_->writeStats();
  }
  std::cout << "write Stats" << std::endl;
}


void writeStats(std::string fname, UInt proc_nodes, UInt total_size, Double total_time, Double ObjVal, Double LowerB, UInt quasi,
     Size_t nlpS, Size_t nlpP, Size_t nlpI, Size_t nlpF)
{
  std::ofstream myfile ("Convex_Results.txt", std::ios::app);

//  myfile.open ("Convex_Results.txt", ios::app );
  if (quasi == 0)
    myfile << fname << std::fixed << ", convex, " << LowerB << "," << ObjVal << "," <<  total_size << ", " << std::fixed << proc_nodes << " ," << std::fixed << total_time << "," << nlpS << "," << nlpP << "," << nlpI << "," << nlpF << std::endl;
  else
    myfile << fname << std::fixed << " ,   quasiconvex,     " << LowerB << "," << ObjVal << ", " << total_size << " , " << std::fixed << proc_nodes << ", " << std::fixed << total_time << "," << nlpS << "," << nlpP << "," << nlpI << "," << nlpF << std::endl;

  myfile.close();
}

void writeStats(std::string fname, std::string methodname, qgStat *myStat, LPEnginePtr lpEng, Double RelTr, 
		Int PoolTr, Double mngrTime, Int RelSize, Int PoolSize, 
		Int RelToPool, Int PoolToRel, Double RelAve, Double PoolAve, quasiQGHandlerPtr qgPtr)
{
  std::stringstream name;
  const char* file_name;
  std::string ss;

  

  //name << "ctmngrRes.txt";
  name << methodname;
  name << "_" << RelTr << "_" << PoolTr << ".txt";
  ss = name.str();
  file_name = ss.c_str();
  std::ofstream myfile(file_name,std::ios::app);
  myfile << fname << "  " << RelTr << " " << PoolTr << " " << myStat->total_time 
         << " " << lpEng->getTime() << " " << myStat->proc_nodes << " " << myStat->LowerB << " " << myStat->ObjVal
	 << " " << qgPtr->QG_strTime() << " " <<  myStat->cuts << " " << myStat->adCuts << " " << myStat->ecpCalls 
	 << std::endl;

  myfile.close();
}

void writeStats(std::string fname, qgStat *myStat, OptionDBPtr options, 
		LPEnginePtr lpEng, quasiQGHandlerPtr qgPtr, Bool ECP)
{
  std::string convexity_type;
  std::string accpm;
  std::string hybrid;
  std::string nlpEngine;
  std::string lpEngine;
  std::string L1L2;
  std::string GL;
  std::stringstream name;
  std::string ss;
  const char* file_name;

  if (myStat->QC){
    convexity_type = "quasiconvex";
    name << "Q";
  }
  else{
    convexity_type = "convex";
    name << "C";
  }

  if (myStat->accpm){
    name << "A";
    accpm = "accpm";
  }
  else{
    name << "N";
    accpm = "no_accpm";
  }
 
  if (myStat->partialFix){
    hybrid = "hybrid";
    name << "H";
  }
  else{
    name << "N";
    hybrid = "no_hybrid";
  }

  if (options->findString("nlp_engine")->getValue()=="IPOPT"){
    nlpEngine = "IPOPT";
    name << "I";
  }
  else {
    nlpEngine = "filter";
    name << "F";
  }

  if (options->findString("lp_engine")->getValue()=="OsiClp"){
    lpEngine = "OsiClp";
    name << "L";
  }
  else if (options->findString("lp_engine")->getValue()=="OsiCpx"){
    lpEngine = "OsiCpx";
    name << "X";
  }

  if (true==options->findBool("GradLin")->getValue())
  {
    name << "G";
    GL = "gradient";
  } else {
    GL = "objective";
    name << "O";
  }

  if (true==options->findBool("l2_project")->getValue()){
    if (ECP==true){
      L1L2 = "L2-B";
      name << "3";
    } else {
      L1L2 = "L2";
      name << "2";
    }    
  } else{
    L1L2 = "L1";
    name << "1"; 
  }

  std::string myhostname = "opt-a012";
  name << ".txt";

  ss = name.str();
  file_name = ss.c_str();
  std::ofstream myfile(file_name,std::ios::app);
  
//  if (true==options->findString("lp_engine")->getValue()=="OsiClp")
//  {
    myfile << fname << " " << "QG" << " " << convexity_type << " " << accpm << " " << hybrid 
		    << " " << nlpEngine << " " << lpEngine << " " << L1L2 << " " << GL 
		    << " " << myStat->LowerB 
                    << " " << myStat->ObjVal << " " << myStat->total_size << " " << myStat->proc_nodes 
                    << " " << myStat->total_time << " "  << myStat->nlpS << " " << myStat->nlpP
                    << " " << myStat->nlpI << " " << myStat->nlpF << " " << myStat->looseCut 
			<< " " << myStat->nlpP1 << " " << myStat->infCuts << " " << myStat->nlpPC
			<< " " << myStat->p2Cuts << " " << myStat->p1Cuts << " " << myStat->CoCuts
			<< " " << myStat->adCuts << " " << lpEng->getTotalIteration() << " " 
			<< qgPtr->QG_strTime() << std::endl;
	//		<< "," << myStat->nlpP1 << "," << myStat->infCuts << "," << myStat->nlpPC <<
	//		 
//	}
//   else if (true==options->findString("lp_engine")->getValue()=="OsiCpx")
//   {
//    myfile << fname << "," << ",QG," << "," << convexity_type << "," << accpm << "," << hybrid 
//		    << "," << OsiCpx << "," << L1L2
//		    << "," << myStat->LowerB 
  //                  << "," << myStat->ObjVal << "," << myStat->total_size << "," << myStat->proc_nodes 
    //                << "," << myStat->total_time << "," << myStat->nlpS << "," << myStat->nlpP
      //              << "," << myStat->nlpI << "," << myStat->nlpF << "," << myStat->looseCut 
	//		<< "," << myStat->nlpP1 << "," << myStat->infCuts << "," << myStat->nlpPC
	//		<< "," << myStat->p2Cuts << "," << myStat->p1Cuts << "," << myStat->CoCuts << std::endl;

//   }
                
}
/*
struct qgStat {
  UInt proc_nodes;
  UInt total_size;
  Double total_time;
  Double ObjVal;
  Double LowerB;
  Size_t nlpS;
  Size_t nlpP;
  Size_t nlpI;
  Size_t nlpF;
  Bool QC;
  Bool partialFix;  
  Bool accpm;
};
*/
void writeSolutionToFile(const Double * x, UInt n)
{ 
  std::ofstream myfile("solution.txt", std::ios::out);
  for (UInt i = 0; i < n; i++)
    myfile << x[i] << std::endl;

  myfile.close();
}
// Local Variables: 
// mode: c++ 
// eval: (c-set-style "k&r") 
// eval: (c-set-offset 'innamespace 0) 
// eval: (setq c-basic-offset 2) 
// eval: (setq fill-column 78) 
// eval: (auto-fill-mode 1) 
// eval: (setq column-number-mode 1) 
// eval: (setq indent-tabs-mode nil) 
// End:
//
//     MINOTAUR -- It's only 1/2 bull

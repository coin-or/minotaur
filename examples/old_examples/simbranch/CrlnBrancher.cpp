//
//    MINOTAUR -- It's only 1/2 bull
//
//    (C)opyright 2008 - 2014 The MINOTAUR Team.
//

/**
 * \file CrlnBrancher.cpp
 * \brief Define methods for  SimRel branching.
 * \edited by Devanand, IIT Bombay
 */

#include <cmath>
#include <iomanip>
#include "MinotaurConfig.h"
#include "Branch.h"
#include "BrCand.h"
#include "BrVarCand.h"
#include "Engine.h"
#include "Environment.h"
#include "Handler.h"
#include "Logger.h"
#include "Modification.h"
#include "Node.h"
#include "Option.h"
#include "ProblemSize.h"
#include "CrlnBrancher.h"
#include "Relaxation.h"
#include "Solution.h"
#include "SolutionPool.h"
#include "Timer.h"
#include "Variable.h"
#include <algorithm>    // std::find_if
#define SPEW 1

using namespace Minotaur;
const std::string RBrDev::me_ = "rbrdev: "; 

RBrDev::RBrDev(EnvPtr env, HandlerVector & handlers) 
: engine_(EnginePtr()),             // NULL
  eTol_(1e-6),
  handlers_(handlers),              // Create a copy, the vector is not too big
  init_(false),
  maxDepth_(100000),
  maxIterations_(25),
  minNodeDist_(0),
  maxStrongCands_(20),
  clmn_(1000),
  rel_(RelaxationPtr()),            // NULL
  status_(NotModifiedByBrancher),
  thresh_(100000),
  trustCutoff_(true),
  maxlookahead_(100),
  x_(0)
{
  timer_ = env->getNewTimer();
  logger_ = (LoggerPtr) new Logger((LogLevel) 
      env->getOptions()->findInt("br_log_level")->getValue());
  stats_ = new RBrDevStats();
  stats_->calls = 0;
  stats_->engProbs = 0;
  stats_->strBrCalls = 0;
  stats_->simCanBr = 0;
  stats_->bndChange = 0;
  stats_->iters = 0;
  stats_->strTime = 0.0;
  stats_->hashTime = 0.0;
  stats_->searchSNodeTime = 0.0;
}

RBrDev::~RBrDev()
{
  delete stats_;
  delete timer_;
  //logger_.reset();
}


BrCandPtr RBrDev::findBestCandidate_(const double objval, double cutoff, NodePtr node, bool* 
					flagsumzero, double* wtdscr,BrCandPtr* wtdcand)
{
    
  double best_score = -INFINITY;
  double score, change_up, change_down, maxchange; 
  UInt cnt, maxcnt,tmpindx,dgrdscr;
  EngineStatus status_up, status_down;
  BrCandPtr cand, best_cand;
  best_cand = BrCandPtr(); // NULL
  int cndindx;

  maxchange = cutoff-objval;
  // do strong branching on branching candidates
  BrCandVIter it;
  if (brnchngCands_.size()>0) {
    BrCandVIter it;
    engine_->enableStrBrSetup();
    engine_->setIterationLimit(maxIterations_); // TODO: make limit dynamic.
    cnt = 0;
    dgrdscr = 0;
    maxcnt = (node->getDepth()>maxDepth_) ? 0 : maxStrongCands_;

    for (it=brnchngCands_.begin(); it!=brnchngCands_.end() 
		    && (cnt < maxcnt) && (dgrdscr < maxlookahead_); ++it, ++cnt) {

      cand = *it;
      strongBranch_(cand, change_up, change_down, status_up, status_down);
      change_up    = std::max((change_up - objval), 0.0);
      change_down  = std::max(change_down - objval, 0.0);
      useStrongBranchInfo_(cand, maxchange, change_up, change_down,status_up, status_down);
      score = getScore_(change_up, change_down);
      cndindx = cand->getPCostIndex();
      // store scores per node 
      tmpindx = crntClmns_*maxStrongCands_ + cnt;
      strongVarIndices_[tmpindx] = cndindx;
      strongInfoDown_[tmpindx] = change_down;
      strongInfoUp_[tmpindx] = change_up;
      lastStrBranched_[cndindx] = stats_->calls;
#if SPEW
      writeScore_(cand, score, change_up, change_down);
#endif
      if (status_!=NotModifiedByBrancher) {
        break;
      }
      if (score > best_score) {
    	dgrdscr = 0;
        best_score = score;
        best_cand = cand;
        if (change_up > change_down) {
          best_cand->setDir(DownBranch);
        } else {
          best_cand->setDir(UpBranch);
        }
      }
      else{
	dgrdscr++;
      }
    }
    engine_->resetIterationLimit(); 
    engine_->disableStrBrSetup();
    
    if (NotModifiedByBrancher == status_) {
      // get score of remaining candidates as well.
      for (;it!=brnchngCands_.end(); ++it) {
        getPCScore_(*it, &change_down, &change_up, &score);
        if (score > best_score){
          best_score = score;
          best_cand = *it;
          if (change_up > change_down){
            best_cand->setDir(DownBranch);
          }
          else {
            best_cand->setDir(UpBranch);
          }
        }
      }
    }
    if(*flagsumzero==true && best_score < *wtdscr && NotModifiedByBrancher == status_)
    {
      best_cand = *wtdcand;
      best_score = *wtdscr;
      ++(stats_->simCanBr); 
    }
  }
#if SPEW
    logger_->msgStream(LogDebug) << me_ << " estimate of change in LB = "
        <<  best_score << std::endl;
#endif


  return best_cand;
}

Branches RBrDev::findBranches(RelaxationPtr rel, NodePtr node, 
                                           ConstSolutionPtr sol,
                                           SolutionPoolPtr s_pool,
                                           BrancherStatus & br_status,
                                           ModVector &mods) 
{

  Branches branches;
  UInt maxcnt;
  int cndindx;
  bool flagsumzero = false;
  double wtdscr;
  BrCandPtr wtdcand = BrCandPtr(); //NULL
  BrCandPtr br_can = BrCandPtr(); //NULL
  const double *x = sol->getPrimal();
  ++(stats_->calls);
  if (!init_) {
    init_ = true;
    initialize(rel);
  }
  rel_ = rel;
  br_status = NotModifiedByBrancher;
  status_ = NotModifiedByBrancher;
  mods_.clear();
  x_.resize(rel->getNumVars());
  std::copy(x, x+rel->getNumVars(), x_.begin());

  // Update table by computing and adding hash values
  timer_->start();
  updateTable_();
  stats_->hashTime += timer_->query();
  timer_->stop();


  //Find candidates
  findCandidates_();
  if (status_ == PrunedByBrancher) {
    br_status = status_;
    return branches;
  }
  if(status_ == NotModifiedByBrancher ){

   // call this to do hash comparison and return the best cand using the similarity concept
    br_can = simNodeHash_(sol->getObjValue(),node->getId(), &flagsumzero, &wtdscr,&wtdcand);

    //either there is a requirement of strong branching calls
    if (br_can==NULL){
      br_can = findBestCandidate_(sol->getObjValue(),
                                   s_pool->getBestSolutionValue(),
                                   node, &flagsumzero,&wtdscr,&wtdcand);
       
      crntClmns_++; //increment the column to record a new info
    }
  }
  if(status_ == NotModifiedByBrancher ){
    branches = br_can->getHandler()->getBranches(br_can, x_, rel_, s_pool); 
      for (BranchConstIterator br_iter=branches->begin(); 
        br_iter!=branches->end(); ++br_iter) {
        (*br_iter)->setBrCand(br_can);
      }
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "best candidate = "
        << br_can->getName() << std::endl;
#endif
  }else {
        // we found some modifications that can be done to the node. Send these
        // back to the processor.
    if (mods_.size()>0) {
      mods.insert(mods.end(),mods_.begin(),mods_.end());
    }
    br_status = status_;
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "found modifications"
                                     << std::endl;
    if (mods_.size()>0) {
      for (ModificationConstIterator miter=mods_.begin(); miter!=mods_.end();
               ++miter)  {
        (*miter)->write(logger_->msgStream(LogDebug));
      }
    } else if (status_==PrunedByBrancher) {
        logger_->msgStream(LogDebug) << me_ << "Pruned." << std::endl;
    } else {
      logger_->msgStream(LogDebug) << me_ << "unexpected status = "
                                     << status_ << std::endl;
    }
#endif
  }
    return branches;
}


void RBrDev::findCandidates_()

{
  VariablePtr v_ptr;
  VariableIterator v_iter, v_iter2, best_iter;
  VariableConstIterator cv_iter;
  int index;
  bool is_inf = false;   // if true, then node can be pruned.

  BrVarCandSet cands;       // candidates from which to choose one.
  BrVarCandSet cands2;      // Temporary set.
  BrCandVector gencands;
  BrCandVector gencands2;   // Temporary vector.
  double s_wt = 1e-5;
  double i_wt = 1e-6;
  double score;

 // first clear the list of candidates
  nvarCands_.clear();
  brnchngCands_.clear();

  for (HandlerIterator h = handlers_.begin(); h != handlers_.end(); ++h) {
    // ask each handler to give some candidates
    (*h)->getBranchingCandidates(rel_, x_, mods_, cands2, gencands2, is_inf);

    for (BrVarCandIter it = cands2.begin(); it != cands2.end(); ++it) {
      (*it)->setHandler(*h);
    }

    for (BrCandVIter it = gencands2.begin(); it != gencands2.end(); ++it) {
      (*it)->setHandler(*h);
    }

    cands.insert(cands2.begin(), cands2.end());
    gencands.insert(gencands.end(), gencands2.begin(), gencands2.end());

    if (is_inf) {
      nvarCands_.clear();
      brnchngCands_.clear();
      cands2.clear();
      cands.clear();
      gencands2.clear();
      gencands.clear();
      status_ = PrunedByBrancher;
      return;
    } else if (mods_.size()>0) {
      status_ = ModifiedByBrancher;
      nvarCands_.clear();
      brnchngCands_.clear();
      cands2.clear();
      cands.clear();
      gencands2.clear();
      gencands.clear();
      return;
    }
    cands2.clear();
    gencands2.clear();
  }

 // visit each candidate in and check if it has reliable pseudo costs.
  for (BrVarCandIter it=cands.begin(); it!=cands.end(); ++it) {
    index = (*it)->getPCostIndex();
    if ((minNodeDist_ > fabs(stats_->calls-lastStrBranched_[index])) &&
      timesUp_[index] >= thresh_ && timesDown_[index] >= thresh_){
         nvarCands_.push_back(*it);
    } else {
      score = timesUp_[index] + timesDown_[index]
         -s_wt*(pseudoUp_[index]+pseudoDown_[index])
         -i_wt*std::max((*it)->getDDist(), (*it)->getUDist());
      (*it)->setScore(score);
      brnchngCands_.push_back(*it);
    } 

  }
  // push all general candidates (that are not variables) as reliable
  // candidates
  for (BrCandVIter it=gencands.begin(); it!=gencands.end(); ++it) {
    nvarCands_.push_back(*it);

  }

  // sort  branching candidates in the increasing order of their reliability.
  std::sort(brnchngCands_.begin(), brnchngCands_.end(), CompareScore);

#if SPEW
  logger_->msgStream(LogDebug) << me_
                               << "number of  branching candidates = " 
                               << brnchngCands_.size() << std::endl;
  if (logger_->getMaxLevel() == LogDebug2) {
    writeScores_(logger_->msgStream(LogDebug2));
  }
#endif
 // for (BrVarCandIter it=cands.begin(); it!=cands.end(); ++it) {
 //   index = (*it)->getPCostIndex();
 // }

  return;
}

bool RBrDev::getTrustCutoff()
{
  return trustCutoff_;
}


UInt RBrDev::getIterLim() 
{
  return maxIterations_;
}


std::string RBrDev::getName() const
{
  return "SimBranch";
}

void RBrDev::getPCScore_(BrCandPtr cand, double *ch_down, 
                                      double *ch_up, double *score) 
{
  int index = cand->getPCostIndex();
  if (index>-1) {
    *ch_down   = cand->getDDist()*pseudoDown_[index];
    *ch_up     = cand->getUDist()*pseudoUp_[index];
    *score     = getScore_(*ch_up, *ch_down);

  } else {
    *ch_down   = 0.0;
    *ch_up     = 0.0;
    *score     = cand->getScore();
  }
}


double RBrDev::getScore_(const double & up_score, 
                                      const double & down_score)
{

  if (up_score>down_score) {
    return down_score*0.6 + up_score*0.4;
  } else {
     return up_score*0.6 + down_score*0.4;
  }
  return 0.;
}

void RBrDev::updateTable_()
    
{
  double tempLb=0.0;
  double tempUb=0.0;
  double sum;

  //if the column of the Matrix reaches clmn_,flush the Matrix  
  if((crntClmns_+1)%clmn_==0){
    resizeNodeInfoDs_(); 
   }    

  //Eval Hash
  for(int h= 0; h <kappa_;h++){
    sum = 0.0;
    for(int i=0; i<numBinary_; i++)
    {
      tempLb = rel_->getVariable(indxBin_[i])->getLb();
      tempUb = rel_->getVariable(indxBin_[i])->getUb();
      sum = sum + (randVal_[h][2*i])*tempLb + (randVal_[h][2*i+1])*tempUb; 
    }
    hashValue_[kappa_*crntClmns_+ h] = sum;
  }
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "hash value1, hash value2 and hash value2 = "
        << hashValue_[kappa_*crntClmns_] <<"\t"<< hashValue_[kappa_*crntClmns_+1]<<
		"\t"<< hashValue_[kappa_*crntClmns_+2] << std::endl;
#endif
}

void RBrDev::resizeNodeInfoDs_(){ 

    hashValue_.resize(kappa_*(clmn_+crntClmns_),0.);//it containts two hashes per node
    NodeIdCollect_.resize(clmn_+crntClmns_,0);
    LbValCollect_.resize(clmn_+crntClmns_,0.);

    strongVarIndices_.resize(maxStrongCands_*(clmn_+crntClmns_),-1);
    strongInfoUp_.resize(maxStrongCands_*(clmn_+crntClmns_),0.0);
    strongInfoDown_.resize(maxStrongCands_*(clmn_+crntClmns_),0.0);
    lbUpdateCntd_.resize(maxStrongCands_*(clmn_+crntClmns_),0);
    lbUpdateCntu_.resize(maxStrongCands_*(clmn_+crntClmns_),0);
  }    


// return postion(indx) of index of element candidate(cndindx) in array strongVarIndices_
bool RBrDev::evalIndx_(UInt clmnindx1, UInt cndindx,int* indx)
{
  UInt start = clmnindx1*maxStrongCands_; 
  UInt end = start + maxStrongCands_; 
  UInt i= 0;
  while(start < end)
  {
    if(strongVarIndices_[start]== cndindx){*indx = i; return true;} 
      start = start+1;
      i++;
  }  
  return false;
}

// AM: Needs simplification and verbose output. Should not return anything.
void RBrDev::mostSimilarNode_()
{
  double hvalprev,hvalcur;
  UInt counter;
#if SPEW
  logger_->msgStream(LogDebug) << me_ <<" current nodeid= " 
                               << NodeIdCollect_[crntClmns_] << " and ids of the similar nodes: "<<"\t";
#endif
  for(unsigned int j=0;j<crntClmns_;j++){
    counter = 0;
    for(unsigned int h=0;h<kappa_;h++){
      hvalcur =hashValue_[kappa_*crntClmns_+ h] ;
      hvalprev = hashValue_[kappa_*j + h];

      if(abs(hvalprev - hvalcur) <=theta_[h])
        counter++;
    }

  if(counter==kappa_){
    binOfSimNodes_.push_back(j);
 #if SPEW
  logger_->msgStream(LogDebug) << NodeIdCollect_[j] <<"\t";
#endif
    }
  }
#if SPEW
  logger_->msgStream(LogDebug)<<"\n"<< me_ << "Number of similar nodes = " 
                               << binOfSimNodes_.size() << std::endl;
#endif
}

BrCandPtr RBrDev::simNodeHash_(double objVl,UInt nodeId, bool* flagsumzero,
                              double* wtdscr,BrCandPtr* wtdcand)
{   

  std::vector<BrCandPtr> brnchngCandstemp;
  UInt cndindx,clmnindx;
  double sumScored,sumScoreu,sumScore; 
  double change_up, change_down,score1;
  double topScore =-INFINITY;
  BrCandPtr matched_cand = BrCandPtr();  // NULL
  BrCandPtr canBstscr = BrCandPtr();     // NULL
  BrCandPtr null_cand = BrCandPtr();     // NULL

  // Update node id and the LB of the current node
  NodeIdCollect_[crntClmns_] = nodeId;
  LbValCollect_[crntClmns_] = objVl;

  // clear the binOfSimNodes_ 
  binOfSimNodes_.clear();

  // if branching call is at root node, do strong branch 
  if (crntClmns_==0) {
     return null_cand;
  }
#if SPEW
  logger_->msgStream(LogDebug) << me_ << "objVal at the current node "
     <<NodeIdCollect_[crntClmns_] <<" is "<< LbValCollect_[crntClmns_]<<std::endl;

  logger_->msgStream(LogDebug) << me_ << "objVal at the parent node "
     <<NodeIdCollect_[crntClmns_-1] <<" is "<< LbValCollect_[crntClmns_-1]<<std::endl;
#endif

  //Compute similar nodes to the current node 
  timer_->start();
  mostSimilarNode_();
  stats_->searchSNodeTime += timer_->query();
  timer_->stop();
  
  // if within a given similarity threshold, we do not find any similar nodes
  // do strong branching
  if(binOfSimNodes_.size()==0 ){
   return null_cand;
  }
  
  //Else, use the similar information 
  for(BrCandVIter it=brnchngCands_.begin(); it!=brnchngCands_.end(); ++it) {

    double alpha=0.80;
    double beta=0.60;
    bool foundflag = false;
    int varindx = 0;
    int simcounter = 0;
    sumScore=0.0;sumScored =0.0;sumScoreu=0.0;
    canBstscr = *it;
    cndindx = canBstscr->getPCostIndex();
    
    for (unsigned int i=0;i<binOfSimNodes_.size();i++) {
      std::vector<unsigned int> tempVctr;
      clmnindx = binOfSimNodes_.at(i);
      if (clmnindx == -1) {
	continue;
      }
      // return true if element cndindx is in array strongVarIndices_,and the index is varindx
      if (evalIndx_(clmnindx, cndindx, &varindx)) {
        sumScored = sumScored + strongInfoDown_[clmnindx*maxStrongCands_ + varindx];
        sumScoreu = sumScoreu + strongInfoUp_[clmnindx*maxStrongCands_ + varindx];
	simcounter++;
        foundflag = true;
      } 
    }
    //variables  which are unavailable in similar nodes, do strong branching: push them into brnchngCandstemp
    if(foundflag==false){
      brnchngCandstemp.push_back(*it);
      *flagsumzero = true;
    }
    else{
      sumScored = sumScored/simcounter; //down-similar score 
      sumScoreu = sumScoreu/simcounter; //up-similar score
    }

    //pseudo-cost score
    getPCScore_(*it, &change_down, &change_up, &score1);
    
    //overall similarity up and down score  
    sumScoreu = alpha*sumScoreu + (1-alpha)*change_up;  
    sumScored = alpha*sumScored + (1-alpha)*change_down;
    
    //total score
    sumScore= beta*std::max(sumScoreu,sumScored)+(1-beta)*std::min(sumScoreu,sumScored);
    
    // find the variable with highest score
    if(topScore<sumScore){
      topScore = sumScore;
      matched_cand =canBstscr ;
      if(sumScoreu > sumScored) {
        matched_cand->setDir(DownBranch);
      } else {
        matched_cand->setDir(UpBranch);
      }
    }
  }
  if (*flagsumzero==true) {
    brnchngCands_.clear();
    brnchngCands_ = brnchngCandstemp;
    brnchngCandstemp.clear();
    *wtdscr = topScore;
    *wtdcand = matched_cand;
    return null_cand;
   } 
 return matched_cand;
}

void RBrDev::bestScoreUpdate_(const double & change_up1,
                             const double & change_down1,
                             const int & indx) {

  std::vector<unsigned int>::iterator it;
  std::vector<unsigned int> tempVctr;
  int varindx = 0;
  UInt tmpindx;

  if(binOfSimNodes_.size()>0){
    for(unsigned int i=0;i<binOfSimNodes_.size();i++){

      if(evalIndx_(i, indx, &varindx)){
        tmpindx = crntClmns_*maxStrongCands_ + varindx;
        if(change_up1 < change_down1){
          strongInfoDown_[tmpindx] = (lbUpdateCntd_[tmpindx]*strongInfoDown_[tmpindx] 
			    + change_down1)/(lbUpdateCntd_[tmpindx]+1);
	  lbUpdateCntd_[tmpindx]++;
	}
	else{
          strongInfoUp_[tmpindx] = (lbUpdateCntu_[tmpindx]*strongInfoUp_[tmpindx] 
			  + change_up1)/(lbUpdateCntu_[tmpindx]+1);
	  lbUpdateCntu_[tmpindx]++;
	 }
      }
    } 
  }
}    
    

UInt RBrDev::getThresh() const
{
  return thresh_;
}


void RBrDev::initialize(RelaxationPtr rel)

{
  int n = rel->getNumVars();
  std::cout<<"total variable"<<n<<std::endl;

  // find number of binaries
  numBinary_ = rel->getSize()->bins;

  // initialize to zero.
  pseudoUp_ = DoubleVector(n,0.); 
  pseudoDown_ = DoubleVector(n,0.); 

  lastStrBranched_ = UIntVector(n,20000);
  timesUp_ = std::vector<UInt>(n,0); 
  timesDown_ = std::vector<UInt>(n,0); 
  //UInt maxsimclmn = std::max(100,int(numBinary_/20));
  //binOfSimNodes_ = UIntVector(maxsimclmn,-1); //currently minimum 50 similar nodes are allowable
  binOfSimNodes_ = UIntVector(1000,-1); //currently minimum 50 similar nodes are allowable
  nvarCands_.reserve(n);
  brnchngCands_.reserve(n);
  x_.reserve(n);
  crntClmns_= 0;

  kappa_ = 3;
  randVal_.resize(kappa_);

  for (int i = 0; i < kappa_; ++i)
    randVal_[i].resize(2*numBinary_,0.);
  //for(int h = 0; h<kappa_;h++){randVal_[h]= DoubleVector(2*numBinary_,0.);}
  
  indxBin_= IntVector(numBinary_,0);
  numBinary_ =0;
  // Eval Weight vectors
  for(int i=0;i<n;i++){
    if(rel->getVariable(i)->getType()==Binary){
      indxBin_[numBinary_] = i;
      for(int j =0; j <kappa_; j++)
      {
	randVal_[j][2*numBinary_] = rand()/(RAND_MAX + 1.);// Lb
        randVal_[j][2*numBinary_+1] = rand()/(RAND_MAX + 1.);//Ub
      }
      numBinary_++;
    }
  }
  //sort the copies of randVals for finding first k elements
  std::vector<double> r[kappa_];
  // Copying  and sorting 
  for(int i = 0; i < kappa_; i++)
  {
     r[i] = randVal_[i];
     std::sort(r[i].begin(), r[i].end(), std::greater<double>());
  }
  // set threshold hamming distance 
  int theta_f = 20; 

  // using 
  theta_= DoubleVector(kappa_,0.); 
  for(int j = 0; j < kappa_; j++){
    for(int i = 0; i < theta_f; i++){theta_[j] += r[j][i];}
  }
  std::cout<<"total binary"<<numBinary_<<"random legth"<<randVal_[1].size()<<"\n";
  std::cout<<"threshold values"<<theta_[0]<<"\t"<<theta_[1]<<"\t"<<theta_[2]<<std::endl; 

  hashValue_= DoubleVector(kappa_*clmn_,0.);
  NodeIdCollect_= UIntVector(clmn_,0);
  LbValCollect_= DoubleVector(clmn_,0.);
  strongVarIndices_= IntVector(maxStrongCands_*clmn_,-1);
  strongInfoUp_= DoubleVector(maxStrongCands_*clmn_,0.);
  strongInfoDown_= DoubleVector(maxStrongCands_*clmn_,0.);
  lbUpdateCntd_= IntVector(maxStrongCands_*clmn_,0);
  lbUpdateCntu_= IntVector(maxStrongCands_*clmn_,0);
}

void RBrDev::setTrustCutoff(bool val)
{
  trustCutoff_ = val;
}


void RBrDev::setEngine(EnginePtr engine)
{
  engine_ = engine;
}


void RBrDev::setIterLim(UInt k) 
{
  maxIterations_ = k;
}


void RBrDev::setMaxDepth(UInt k) 
{
  maxDepth_ = k;
}


void RBrDev::setMinNodeDist(UInt k) 
{
  minNodeDist_ = k;
}


void RBrDev::setThresh(UInt k) 
{
  thresh_ = k;
}


bool RBrDev::shouldPrune_(const double &chcutoff, const double &change,
                          const EngineStatus & status, bool *is_rel)
{
  switch (status) {
   case (ProvenLocalInfeasible):
     return true;
   case (ProvenInfeasible):
     return true;
   case (ProvenObjectiveCutOff):
     return true;
   case (ProvenLocalOptimal):
   case (ProvenOptimal):
     if (trustCutoff_ && change>chcutoff-eTol_) {
       return true;
     }
     // check feasiblity
     break;
   case (EngineUnknownStatus):
     assert (!"engine status is UnknownStatus in reliability branching!");
     break;
   case (EngineIterationLimit):
     break;
   case (ProvenFailedCQFeas):
   case (ProvenFailedCQInfeas):
     logger_->msgStream(LogInfo) << me_ << "Failed CQ. Continuing."
                                 << std::endl;
     *is_rel = false;
     break;
   default:
     logger_->errStream() << me_ << "unexpected engine status. " 
                          << "status = " << status << std::endl;
     *is_rel = false;
     stats_->engProbs += 1;
     break;
  }
  return false;
}


void RBrDev::strongBranch_(BrCandPtr cand, double & obj_up, double & obj_down, 
                           EngineStatus & status_up, 
                           EngineStatus & status_down)
{
  HandlerPtr h = cand->getHandler();
  ModificationPtr mod;

  // first do down.
  mod = h->getBrMod(cand, x_, rel_, DownBranch);
  mod->applyToProblem(rel_);
  //std::cout << "down relax ******\n";
  //rel_->write(std::cout);

  timer_->start();
  status_down = engine_->solve();
  stats_->strTime += timer_->query();
  timer_->stop();
  ++(stats_->strBrCalls);
  obj_down = engine_->getSolutionValue();
  mod->undoToProblem(rel_);

  // now go up.
  mod = h->getBrMod(cand, x_, rel_, UpBranch);
  mod->applyToProblem(rel_);
  //std::cout << "up relax ******\n";
  //rel_->write(std::cout);

  timer_->start();
  status_up = engine_->solve();
  stats_->strTime += timer_->query();
  timer_->stop();
  ++(stats_->strBrCalls);
  obj_up = engine_->getSolutionValue();
  mod->undoToProblem(rel_);
}


void RBrDev::updateAfterSolve(NodePtr node, ConstSolutionPtr sol)
{
  const double *x = sol->getPrimal();
  NodePtr parent = node->getParent();
  if (parent) {
    BrCandPtr cand = node->getBranch()->getBrCand();
    int index = cand->getPCostIndex();
    if (index>-1) {
      double oldval = node->getBranch()->getActivity();
      double newval = x[index];
      double cost = (node->getLb()-parent->getLb()) / 
                    (fabs(newval - oldval)+eTol_);
      if (cost < 0. || std::isinf(cost) || std::isnan(cost)) {
        cost = 0.;
      }
      if (newval < oldval) {
        updatePCost_(index, cost, pseudoDown_, timesDown_);
        bestScoreUpdate_(0.0, cost, index); 
      } else {
        updatePCost_(index, cost, pseudoUp_, timesUp_);   
        bestScoreUpdate_(cost, 0.0, index); 
      }
    } 
  }
}


void RBrDev::updatePCost_(const int & i, const double & new_cost, 
                                       DoubleVector & cost, UIntVector & count)
{
  cost[i]   = (cost[i]*count[i]+new_cost)/(count[i]+1);
  count[i] += 1;
}


void RBrDev::useStrongBranchInfo_(BrCandPtr cand, const double &chcutoff, 
                                  double &change_up, double &change_down,
                                  const EngineStatus & status_up,
                                  const EngineStatus & status_down)
{
  const UInt index        = cand->getPCostIndex();
  bool should_prune_up    = false;
  bool should_prune_down  = false;
  bool is_rel = true;
  double cost;

  should_prune_down = shouldPrune_(chcutoff, change_down, status_down, &is_rel);
  should_prune_up   = shouldPrune_(chcutoff, change_up, status_up, &is_rel);

  if (!is_rel) {
    change_up = 0.;
    change_down = 0.;
  } else if (should_prune_up == true && should_prune_down == true) {
    status_ = PrunedByBrancher;
    stats_->bndChange += 2;
  } else if (should_prune_up) {
    status_ = ModifiedByBrancher;
    mods_.push_back(cand->getHandler()->getBrMod(cand, x_, rel_, DownBranch));
    ++(stats_->bndChange);
  } else if (should_prune_down) {
    status_ = ModifiedByBrancher;
    mods_.push_back(cand->getHandler()->getBrMod(cand, x_, rel_, UpBranch));
    ++(stats_->bndChange);
  } else { 
    cost = fabs(change_down)/(fabs(cand->getDDist())+eTol_);
    updatePCost_(index, cost, pseudoDown_, timesDown_);
    //bestScoreUpdate_(0.0, cost, index); 

    cost = fabs(change_up)/(fabs(cand->getUDist())+eTol_);
    updatePCost_(index, cost, pseudoUp_, timesUp_);
    //bestScoreUpdate_(cost, 0.0, index); 
  }
}


void RBrDev::writeScore_(BrCandPtr cand, double score, 
                                      double change_up, double change_down)
{
  logger_->msgStream(LogDebug2) << me_ << "candidate: " << cand->getName() 
                                << " down change = " << change_down
                                << " up change = " << change_up
                                << " score = " << score
                                << std::endl;
}


void RBrDev::writeScores_(std::ostream &out)
{
  out << me_ << " branching candidates:" << std::endl;
  for (BrCandVIter it=brnchngCands_.begin(); it!=brnchngCands_.end(); ++it) {
    if ((*it)->getPCostIndex()>-1) {
      out << std::setprecision(6) << (*it)->getName() << "\t" 
        << timesDown_[(*it)->getPCostIndex()] << "\t"
        << timesUp_[(*it)->getPCostIndex()] << "\t" 
        << pseudoDown_[(*it)->getPCostIndex()] << "\t"
        << pseudoUp_[(*it)->getPCostIndex()] << "\t"
        << x_[(*it)->getPCostIndex()] << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getLb() << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getUb() << "\t"
        << std::endl;
    } else {
      out << std::setprecision(6) << (*it)->getName() << "\t" 
                                  << 0 << "\t" << 0 << "\t"
                                  << (*it)->getScore() << "\t"
                                  << (*it)->getScore() << "\t"
                                  << (*it)->getDDist() << "\t"
                                  << 0.0 << "\t"
                                  << 1.0 << "\t" << std::endl;
    }
  }

  out << me_ << "reliable candidates:" << std::endl;
  for (BrCandVIter it=nvarCands_.begin(); it!=nvarCands_.end(); ++it) {
    if ((*it)->getPCostIndex()>-1) {
      out << (*it)->getName() << "\t" 
        << timesDown_[(*it)->getPCostIndex()] << "\t"
        << timesUp_[(*it)->getPCostIndex()] << "\t" 
        << pseudoDown_[(*it)->getPCostIndex()] << "\t"
        << pseudoUp_[(*it)->getPCostIndex()] << "\t"
        << x_[(*it)->getPCostIndex()] << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getLb() << "\t"
        << rel_->getVariable((*it)->getPCostIndex())->getUb() << "\t"
        << std::endl;
    } else {
      out << std::setprecision(6) << (*it)->getName() << "\t" 
          << 0 << "\t" << 0 << "\t"
          << (*it)->getScore() << "\t"
          << (*it)->getScore() << "\t"
          << (*it)->getDDist() << "\t"
          << 0.0 << "\t"
          << 1.0 << "\t" << std::endl;
    }
  }
}


void RBrDev::writeStats(std::ostream &out) const
{
  if (stats_) {
    out << me_ << "times called                = " << stats_->calls 
      << std::endl
      << me_ << "no. of problems in engine   = " << stats_->engProbs
      << std::endl
      << me_ << "times relaxation solved     = " << stats_->strBrCalls
      << std::endl
      << me_ << "times bounds changed        = " << stats_->bndChange
      << std::endl
      << me_ << "time in solving relaxations = " << stats_->strTime
      << std::endl
      << me_ << "time in hashing operation = " << stats_->hashTime
      << std::endl
      << me_ << "time in searching simiar nodes  = " << stats_->searchSNodeTime
      << std::endl
      << me_ << "no. of times best cand is selected by sim explored nodes (not from strng brnchng) = " << stats_->simCanBr
      << std::endl;
  }
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

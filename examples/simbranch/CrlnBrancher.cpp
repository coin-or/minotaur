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
#define SPEW 0

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
  rel_(RelaxationPtr()),            // NULL
  status_(NotModifiedByBrancher),
  thresh_(10000),
  trustCutoff_(true),
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
  logger_.reset();
}


BrCandPtr RBrDev::findBestCandidate_(const double objval, double cutoff, NodePtr node, bool* 
					flagsumzero, double* wtdscr,BrCandPtr* wtdcand)
{
    
  double best_score = -INFINITY;
  double score, change_up, change_down, maxchange; 
  UInt cnt, maxcnt,tmpindx;
  EngineStatus status_up, status_down;
  BrCandPtr cand, best_cand,canBstscr;
  best_cand = BrCandPtr(); // NULL
  int cndindx;

  maxchange = cutoff-objval;
  // do strong branching on branching candidates

  if (brnchngCands_.size()>0) {
    BrCandVIter it;
    engine_->enableStrBrSetup();
    engine_->setIterationLimit(maxIterations_); // TODO: make limit dynamic.
    cnt = 0;
    maxcnt = (node->getDepth()>maxDepth_) ? 0 : maxStrongCands_;

    for (it=brnchngCands_.begin(); it!=brnchngCands_.end() && cnt < maxcnt ; ++it, ++cnt) {
      cand = *it;
      strongBranch_(cand, change_up, change_down, status_up, status_down);
      change_up    = std::max((change_up - objval), 0.0);
      change_down  = std::max(change_down - objval, 0.0);
      useStrongBranchInfo_(cand, maxchange, change_up, change_down,status_up, status_down);
      score = getScore_(change_up, change_down);
      canBstscr =*it;
      cndindx = canBstscr->getPCostIndex();
      // store scores per node 
      tmpindx = crntClmns_*maxStrongCands_ + cnt;
      scoreIndxMat_[tmpindx] = cndindx;
      scoreMatd_[tmpindx] = change_down;
      scoreMatu_[tmpindx] = change_up;
      lastStrBranched_[cand->getPCostIndex()] = stats_->calls;
#if SPEW
      writeScore_(cand, score, change_up, change_down);
#endif
      if (status_!=NotModifiedByBrancher) {
        break;
      }
      if (score > best_score) {
        best_score = score;
        best_cand = cand;
        if (change_up > change_down) {
          best_cand->setDir(DownBranch);
        } else {
          best_cand->setDir(UpBranch);
        }
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
  findCandidates_();
  if (status_ == PrunedByBrancher) {
    br_status = status_;
     return branches;
  }
  if(status_ == NotModifiedByBrancher ){

   // call this to eval hash, do hash comparison and return the cand using similar nodes concept
    br_can = simNodeHash(sol->getObjValue(),node->getId(), &flagsumzero, &wtdscr,&wtdcand);
    if (br_can==NULL){
       br_can = findBestCandidate_(sol->getObjValue(),
                                   s_pool->getBestSolutionValue(),
                                   node, &flagsumzero,&wtdscr,&wtdcand);
       
       crntClmns_++; //increment the column as the current node is not getting sufficient info, so record new info
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
    } else {
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
               ++miter) {
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
  return "RBrDev";
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
    return down_score*0.8 + up_score*0.2;
  } else {
     return up_score*0.8 + down_score*0.2;
  }
  return 0.;
}

void RBrDev::updateTable_(const double & objVl)
    
{
  double bndwt=.5;
  int bandsize;
  double tempFeature=0.0;
  //if the column of the Matrix increased to clmn_,flush the Matrix  
  if((crntClmns_+1)%clmn_==0){
    crntClmns_ = 0;   
  }    
  //featureMat_[0][crntClmns_] =objVl;
  // Compute band size
   
   // flush before storing it   
   hashValue_[hash_*crntClmns_] =0.0;
   hashValue_[hash_*crntClmns_+1] =0.0;


  for(int i=0; i<numBinary_; i++)
  {
    tempFeature = bndwt*(rel_->getVariable(indxBin_[i])->getLb()) +
        (1-bndwt)*rel_->getVariable(indxBin_[i])->getUb();
    hashValue_[hash_*crntClmns_] = hashValue_[hash_*crntClmns_] + (randVal1_[i])*tempFeature;
    hashValue_[hash_*crntClmns_+1] = hashValue_[hash_*crntClmns_+1] + (randVal2_[i])*tempFeature;
  }
#if SPEW
        logger_->msgStream(LogDebug) << me_ << "hash value1 and hash value2 = "
        << hashValue_[hash_*crntClmns_] <<"\t"<< hashValue_[hash_*crntClmns_+1] << std::endl;
#endif
}
// return postion(indx) of index of candidate(cndindx) in scoreIndxMat_
bool RBrDev::evalIndx(UInt clmnindx1, UInt cndindx,int* indx)
{
  UInt start = clmnindx1*maxStrongCands_; 
  UInt end = start + maxStrongCands_; 
  UInt i= 0;
  while(start < end)
   {
   if(scoreIndxMat_[start]== cndindx){*indx = i; return true;} 
   start = start+1;
   i++;

   }  
  return false;
}

// AM: Needs simplification and verbose output. Should not return anything.
void RBrDev::mostSimilarNode()
{
  double hvalprev,hvalcur;
  double thrshld = std::min(0.15,(20./numBinary_));
  //std::vector<UInt> occurNodeId(crntClmns_,0);
  UInt counter;
#if SPEW
  logger_->msgStream(LogDebug) << me_ <<" current nodeid= " 
                               << NodeIdCollect_[crntClmns_] << " and ids of the similar noded: "<<"\t";
#endif

  //if a node is available in every bin then it is similar 
  for(unsigned int j=0;j<crntClmns_;j++){
    counter = 0;
    for(unsigned int h=0;h<hash_;h++){
      hvalcur =hashValue_[hash_*crntClmns_+ h] ;
      hvalprev = hashValue_[hash_*j + h];
      if(hvalprev >= hvalcur*(1-thrshld) && hvalprev <= hvalcur*(1+thrshld))
        counter++;
    }
  if(counter>hash_-1){
    binrslt_.push_back(j);
 #if SPEW
  logger_->msgStream(LogDebug) << NodeIdCollect_[j] <<"\t";
#endif
    }
  }

#if SPEW
  logger_->msgStream(LogDebug)<<"\n"<< me_ << "Number of similar nodes = " 
                               << binrslt_.size() << std::endl;
#endif

}

BrCandPtr RBrDev::simNodeHash(double objVl,UInt nodeId, bool* flagsumzero,
                              double* wtdscr,BrCandPtr* wtdcand)
{   

  std::vector<BrCandPtr> brnchngCandstemp;
  UInt cndindx,clmnindx;
  double sumScored = 0.0; 
  double sumScoreu = 0.0;
  double sumScore = 0.0;
  double change_up, change_down,score1;
  double topScore =-INFINITY;
  BrCandPtr matched_cand = BrCandPtr();  // NULL
  BrCandPtr canBstscr = BrCandPtr();     // NULL
  BrCandPtr null_cand = BrCandPtr();     // NULL

  // Update node id and the LB of the current node
  NodeIdCollect_[crntClmns_] = nodeId;
  LbValCollect_[crntClmns_] = objVl;

  // clear the binrslt_ 
  binrslt_.clear();
  // Update table by computing  hash values
  timer_->start();
  updateTable_(objVl);
  stats_->hashTime += timer_->query();
  timer_->stop();

  // if branching call is at root node, do strong branch 
  if (crntClmns_==0) {
     return null_cand;
  }
#if SPEW
    logger_->msgStream(LogDebug) << me_ << "objVal at the current node "
       <<NodeIdCollect_[crntClmns_] <<" is "<< LbValCollect_[crntClmns_]<<std::endl;

    logger_->msgStream(LogDebug) << me_ << "objVal at the node "
       <<NodeIdCollect_[crntClmns_-1] <<" is "<< LbValCollect_[crntClmns_-1]<<std::endl;
#endif

  //Compute similar nodes to the current node 
  timer_->start();
  mostSimilarNode();
  stats_->searchSNodeTime += timer_->query();
  timer_->stop();

  // if within a given similarity threshold, we do not find any similar nodes
  // do strong branching
  if(binrslt_.size()==0 ){
   return null_cand;
  }
  
  //Else, use the similar information 
  for(BrCandVIter it=brnchngCands_.begin(); it!=brnchngCands_.end(); ++it) {
    canBstscr = *it;
    cndindx = canBstscr->getPCostIndex();
    double scr_wt=0.85 ;
    bool foundflag = false;
    int varindx = 0;
    for (unsigned int i=0;i<binrslt_.size();i++) {
      std::vector<unsigned int> tempVctr;
      clmnindx = binrslt_.at(i);
      if (clmnindx == -1) {continue;}
     // std::copy(scoreIndxMat_.begin() + clmnindx*maxStrongCands_, scoreIndxMat_.begin() +
//		 (clmnindx+1)*maxStrongCands_, std::back_inserter(tempVctr));

      if (evalIndx(clmnindx, cndindx, &varindx)) {
//	std::cout<<"indexing "<<varindx<<" prev clmn number "<<clmnindx << "current clmn number "<< crntClmns_<<std::endl;
        sumScored = sumScored + scoreMatd_[clmnindx*maxStrongCands_ + varindx];
        sumScoreu = sumScoreu + scoreMatu_[clmnindx*maxStrongCands_ + varindx];
        foundflag = true;
      } 
    }
    //there might be some variables  which is not available in similar nodes, do
    //strong branching
    if(foundflag==false){
      brnchngCandstemp.push_back(*it);
      *flagsumzero = true;
    }
    else{
      sumScored = sumScored/binrslt_.size();
      sumScoreu = sumScoreu/binrslt_.size();
    }

    getPCScore_(*it, &change_down, &change_up, &score1);
    //Compute the weighted average
    sumScore= scr_wt*(getScore_(sumScoreu, sumScored))+(1-scr_wt)*score1;
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
    sumScore=0.0;sumScored =0.0;sumScoreu=0.0;
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

void RBrDev::bestScoreUpdate(const double & change_up1,
                             const double & change_down1,
                             const int & indx) {

  std::vector<unsigned int>::iterator it;
  std::vector<unsigned int> tempVctr;
  double wtdavg=0.5;
  int varindx = 0;
  UInt tmpindx;
  if(binrslt_.size()>0){
    for(unsigned int i=0;i<binrslt_.size();i++){
     // std::copy(scoreIndxMat_.begin() +i*maxStrongCands_, scoreIndxMat_.begin() + 
     //           (i+1)*maxStrongCands_, std::back_inserter(tempVctr) );

      if(evalIndx(i, indx, &varindx)){
        tmpindx = crntClmns_*maxStrongCands_ + varindx;
        if(change_up1 < change_down1){
          scoreMatd_[tmpindx] = (lbUpdateCntd_[tmpindx]*scoreMatd_[tmpindx] 
			    + change_down1)/(lbUpdateCntd_[tmpindx]+1);
	  lbUpdateCntd_[tmpindx]++;
	}
	else{
          scoreMatu_[tmpindx] = (lbUpdateCntu_[tmpindx]*scoreMatu_[tmpindx] 
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
  // find number of binaries
  numBinary_ = rel->getSize()->bins;
  // initialize to zero.
  pseudoUp_ = DoubleVector(n,0.); 
  pseudoDown_ = DoubleVector(n,0.); 
  lastStrBranched_ = UIntVector(n,20000);
  timesUp_ = std::vector<UInt>(n,0); 
  timesDown_ = std::vector<UInt>(n,0); 
  clmn_= 1000; //change made from 150 to 100 see SimpledevCrln_16_2_reduced_limit_threshold
  UInt maxsimclmn = std::max(100,int(numBinary_/20));
  binrslt_ = UIntVector(maxsimclmn,-1); //currently minimum 50 similar nodes are allowable
  nvarCands_.reserve(n);
  brnchngCands_.reserve(n);
  x_.reserve(n);
  varlen_ = n;
  crntClmns_= 0;
  hash_ = 2;
  randVal1_= DoubleVector(numBinary_,0.);
  randVal2_= DoubleVector(numBinary_,0.);
  indxBin_= IntVector(numBinary_,0);
  numBinary_ =0;
  for(int i=0;i<n;i++){
    if(rel->getVariable(i)->getType()==Binary){
      indxBin_[numBinary_] = i;
      randVal1_[numBinary_] = rand()/(RAND_MAX + 1.);  
      randVal2_[numBinary_] = rand()/(RAND_MAX + 1.);  
      numBinary_++;
    }
  }
  hashValue_= DoubleVector(2*clmn_,0.);//it containts two hashes per node
  NodeIdCollect_= UIntVector(clmn_,0);
  LbValCollect_= DoubleVector(clmn_,0.);
  scoreIndxMat_= IntVector(maxStrongCands_*clmn_,-1);
  scoreMatu_= DoubleVector(maxStrongCands_*clmn_,0.);
  scoreMatd_= DoubleVector(maxStrongCands_*clmn_,0.);
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
        bestScoreUpdate(0.0, cost, index); // AM: why pass simtimesDown_?
      } else {
        updatePCost_(index, cost, pseudoUp_, timesUp_);   
        bestScoreUpdate(cost, 0.0, index);   // AM: why pass simtimesUp_?
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

    cost = fabs(change_up)/(fabs(cand->getUDist())+eTol_);
    updatePCost_(index, cost, pseudoUp_, timesUp_);
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

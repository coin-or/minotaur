//
//     MINOTAUR -- It's only 1/2 bull
//
//     (C)opyright 2008--2010 The MINOTAUR Team
//

/**
 This is example driver code to take a problem and RLTize it
*/

// Put config stuff (by hand) here.  Why?
#define MINOTAUR_RUSAGE

//#include "MinotaurConfig.h"

#include <AMPLInterface.h>
#undef filename
#include <MultilinearHandler.h>
#include <Problem.h>
#include <LPRelaxation.h>
#include <OsiLPEngine.h>
#include <Operations.h>
#include <Function.h>
#include <vector>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>
#include <iostream>
#include <LinearFunction.h>
#include <QuadraticFunction.h>
#include <PolynomialFunction.h>
#include <Variable.h>
#include <Environment.h>
#include <Constraint.h>
#include <Objective.h>
#include <Solution.h>

using namespace Minotaur;
using namespace std;
static long t_start, t_finish;
static struct tms t_usage;
static double timeTaken = 0.0;
static long int clktck = sysconf (_SC_CLK_TCK);
static double timelimit = 1000000.00;


void time_start(){

  times (&t_usage);
  t_start = t_usage.tms_utime + t_usage.tms_stime;
 
  //printf("TT: %g\n", timeTaken );
}

void time_finish(){
 
  times (&t_usage);
  t_finish = t_usage.tms_utime + t_usage.tms_stime;    
  timeTaken += (double)(t_finish - t_start)/clktck;
  //printf("TT: %g\n", timeTaken );
}

bool time_limit(){
 
  times (&t_usage);
  t_finish= t_usage.tms_utime + t_usage.tms_stime;
  //printf("TT: %g\n", timeTaken );
  if(timeTaken + (double)(t_finish - t_start)/clktck > timelimit)
    return true;
  return false;

}

double time_taken(){
 
  times (&t_usage);
  t_finish= t_usage.tms_utime + t_usage.tms_stime;
  // printf("TT: %g\n", timeTaken );
  return (timeTaken + (double)(t_finish - t_start)/clktck);

}

void visit(std::vector<int> &S, std::vector<double> &lb,
                       std::vector<double> &ub, int ix, std::vector<double> &val,
                       std::vector<std::vector<double> > &E)
{
  if (ix == S.size()) {
    E.push_back(val);
    return ;
  }
  val[ix] = lb[S[ix]];
  visit(S,lb,ub,ix+1,val,E);

  val[ix] = ub[S[ix]];
  visit(S,lb,ub,ix+1,val,E);

}

void allExtreme(std::vector<int> &S, std::vector<double> &lb,
                               std::vector<double> &ub, std::vector<std::vector<double> > &E)
{
  std::vector<double> val(S.size());
  int ix = 0;
  visit(S, lb, ub, ix, val, E);

#if 0
  for(UInt i = 0; i < E.size(); i++) {
    for(UInt j = 0; j < E[i].size(); j++) {
      std::cout << E[i][j] << ",";
    }
    std::cout << std::endl;
  }
#endif


}

class relaxationInfo {
public:
  vector<vector<ConstVariablePtr> > groups;
  vector<vector<ConstVariablePtr> > new_groups;
  vector<vector<ConstVariablePtr> > add_groups;
  vector<vector<ConstVariablePtr> > all_lambdas;
  vector<vector<ConstVariablePtr> > new_all_lambdas;
  std::map <ConstVariablePtr, ConstVariablePtr> oVars;
  std::map <ConstVariablePtr, ConstVariablePtr> rev_oVars;
  std::map<ConstVariablePtr, std::vector<ConstVariablePtr> > newCopyVariables;
  std::map<ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator newCopyVariables_it;
  std::map <ConstVariablePtr, ConstVariablePair> sqterms;
  std::map <ConstVariablePair, ConstVariablePtr> rev_sqterms;
  std::map <ConstVariablePtr, ConstVariablePair> blterms;
  std::map <ConstVariablePair, ConstVariablePtr> rev_blterms;
  std::map <VarIntMap, ConstVariablePtr> monomial_terms;
  std::map <VarIntMap, ConstVariablePtr>::iterator monomial_terms_it;
  std::map <ConstVariablePtr, std::vector<ConstVariablePtr> > mlterms;
  std::map <ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator mlterms_it;
  std::map <std::vector<ConstVariablePtr>, ConstVariablePtr > rev_mlterms;
  std::map <ConstVariablePtr, int> max_pow;
  std::map <ConstVariablePtr, int>::iterator max_pow_it;

  // New maps
  std::map <ConstVariablePtr, ConstVariablePair> new_sqterms;
  std::map <ConstVariablePair, ConstVariablePtr> new_rev_sqterms;
  std::map <ConstVariablePtr, ConstVariablePair> new_blterms;
  std::map <ConstVariablePair, ConstVariablePtr> new_rev_blterms;
  std::map <ConstVariablePtr, ConstVariablePair> add_sqterms;
  std::map <ConstVariablePair, ConstVariablePtr> add_rev_sqterms;
  std::map <ConstVariablePtr, ConstVariablePair> add_blterms;
  std::map <ConstVariablePair, ConstVariablePtr> add_rev_blterms;
  std::map <ConstVariablePtr, std::vector<ConstVariablePtr> > new_mlterms;
  std::map <ConstVariablePtr, std::vector<ConstVariablePtr> > add_mlterms;
  std::map <std::vector<ConstVariablePtr>, ConstVariablePtr > new_rev_mlterms;
  std::map <std::vector<ConstVariablePtr>, ConstVariablePtr > add_rev_mlterms;
  std::map <VarIntMap, ConstVariablePtr> new_monomial_terms;
  std::map <VarIntMap, ConstVariablePtr> add_monomial_terms;
  std::map <ConstVariablePtr, int> new_max_pow;
  std::map <ConstVariablePtr, int>::iterator new_max_pow_it;
  std::map<ConstVariablePtr, std::vector<ConstVariablePtr> > new_newCopyVariables;

  std::map <ConstVariablePtr, ConstVariablePair> blterms_tobe_regrouped;
  std::map <ConstVariablePair, ConstVariablePtr> rev_blterms_tobe_regrouped;
  std::map <ConstVariablePtr, std::vector<ConstVariablePtr> > mlterms_tobe_regrouped;
  std::map <std::vector<ConstVariablePtr>, ConstVariablePtr > rev_mlterms_tobe_regrouped;

  void setInitialNewMaps();
  void updateNewMaxPow(QuadraticFunctionPtr q, PolyFunPtr p, LPRelaxationPtr &lp);
  void updateNewNewCopyVariables(LPRelaxationPtr &lp);
  void linearizeQuadraticRltf(QuadraticFunctionPtr q, LinearFunctionPtr &l, LPRelaxationPtr &lp, Const Double *x);
  void linearizePolynomialRltf(PolyFunPtr p, LinearFunctionPtr &l, LPRelaxationPtr &lp);
  void buildAdditionalTermsByGroups(LPRelaxationPtr &lp);
  void assignNewMapsToOldMaps();
  void assignOldMapsToNewMaps();
};

void relaxationInfo::setInitialNewMaps() {
  // new_groups = groups
  for(int it=0; it < groups.size(); it++) {
    std::vector<ConstVariablePtr> additional_group;
    for (std::vector<ConstVariablePtr>::iterator it1 = groups[it].begin(); 
	 it1 != groups[it].end(); ++it1) {
      additional_group.push_back(*it1);
    }
    new_groups.push_back(additional_group);
  }

  // new_all_lambdas = all_lambdas;
  for(vector<vector<ConstVariablePtr> >::iterator it = all_lambdas.begin();
      it != all_lambdas.end(); ++it) {
    vector<ConstVariablePtr> lambda_variables;
    for(int it1=0; it1<(*it).size(); it1++) {
      lambda_variables.push_back((*it)[it1]);
    }
    new_all_lambdas.push_back(lambda_variables);
  }
		  
  // new_newCopyVariables = newCopyVariables
  for(map<ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator it = newCopyVariables.begin();
      it != newCopyVariables.end(); ++it) {
    new_newCopyVariables.insert(*it);
  }
		
  // new_sqterms = sqterms
  for(map <ConstVariablePtr, ConstVariablePair>::iterator it = sqterms.begin();
      it != sqterms.end(); ++it) {
    new_sqterms.insert(*it);
  }
		  
  // new_rev_sqterms = rev_sqterms
  for(map <ConstVariablePair, ConstVariablePtr>::iterator it = rev_sqterms.begin();
      it != rev_sqterms.end(); ++it) {
    new_rev_sqterms.insert(*it);
  }
		  
  // new_blterms = blterms
  for(map <ConstVariablePtr, ConstVariablePair>::iterator it = blterms.begin();
      it != blterms.end(); ++it) {
    new_blterms.insert(*it);
  }

  // new_rev_blterms = rev_blterms
  new_rev_blterms.clear();
  for(map <ConstVariablePair, ConstVariablePtr>::iterator it = rev_blterms.begin();
      it != rev_blterms.end(); ++it) {
    new_rev_blterms.insert(*it);
  }

  // new_monomial_terms = monomial_terms
  for(map <VarIntMap, ConstVariablePtr>::iterator it = monomial_terms.begin();
      it != monomial_terms.end(); ++it) {
    new_monomial_terms.insert(*it);
  }
		
  // new_mlterms = mlterms
  for(map <ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator it=mlterms.begin();
      it != mlterms.end(); ++it) {
    new_mlterms.insert(*it);
  }
		
  // new_rev_mlterms = rev_mlterms
  for(map <std::vector<ConstVariablePtr>,ConstVariablePtr >::iterator it=rev_mlterms.begin();
      it != rev_mlterms.end(); ++it) {
    new_rev_mlterms.insert(*it);
  }
		
  // new_max_pow = max_pow
  for(map <ConstVariablePtr, int>::iterator it = max_pow.begin();
      it != max_pow.end(); ++it) {
    new_max_pow.insert(*it);
  }
}

void relaxationInfo::updateNewMaxPow(QuadraticFunctionPtr q, PolyFunPtr p, LPRelaxationPtr &lp) {
  // update the new_max_pow map for the quadratic part
  if(q) {
    for(VariablePairGroupConstIterator qit = q->begin(); qit != q->end(); ++qit) {
      ConstVariablePtr x1 = qit->first.first;
      ConstVariablePtr x2 = qit->first.second;
      max_pow_it = max_pow.find(oVars.find(x1)->second); 
      
      if(x1->getId()==x2->getId()) {
	if((max_pow.find(oVars.find(x1)->second))->second < 2 ) {
	  (new_max_pow.find(oVars.find(x1)->second))->second = 2;
	}
      }
    }
  }
  
  // For the polynomial part update the max_pow and make a list of monomial terms
  if(p) {
    for(MonomialConstIter mit = p->termsBegin(); mit != p->termsEnd(); ++mit) {
      const VarIntMap *mit_terms = (*mit)->getTerms();
      if(monomial_terms.find(*mit_terms) == monomial_terms.end()) {
	// Find the bounds on the monomial
	double lb1 = 1;
	double ub1 = 1;
	double lb = 0;
	double ub = 0;
	for(VarIntMapConstIterator tit = (*mit)->termsBegin(); tit != (*mit)->termsEnd(); ++tit) {
	  for(int i = 1; i <= tit->second; i++) {
	    BoundsOnProduct((tit->first)->getLb(), (tit->first)->getUb(), lb1, ub1, lb, ub);
	    lb1 = lb;
	    ub1 = ub;
	  }
	}
	
	ConstVariablePtr y = lp->newVariable(lb, ub, Continuous);
	new_monomial_terms.insert(std::pair<VarIntMap, ConstVariablePtr>(*mit_terms, y));
	add_monomial_terms.insert(std::pair<VarIntMap, ConstVariablePtr>(*mit_terms, y));
      }
      for(VarIntMapConstIterator tit = (*mit)->termsBegin(); tit != (*mit)->termsEnd(); ++tit) {
	new_max_pow_it = new_max_pow.find((oVars.find(tit->first))->second);
	if(new_max_pow_it->second < tit->second && tit->second >= 2) { 
	  new_max_pow_it->second = tit->second;
	}
      }
    }
  }  
}

void relaxationInfo::updateNewNewCopyVariables(LPRelaxationPtr &lp) {
  for(newCopyVariables_it = new_newCopyVariables.begin(); newCopyVariables_it != new_newCopyVariables.end(); 
      ++newCopyVariables_it) {
    int oldPow = (max_pow.find(newCopyVariables_it->first))->second;
    int newPow = (new_max_pow.find(newCopyVariables_it->first))->second;
    if(oldPow < newPow) {
      int max_i;
      if (oldPow == 0)
	max_i = newPow - oldPow - 1;
      else
	max_i = newPow - oldPow;
      for(int i=1; i <= max_i ; i++) {
	ConstVariablePtr v = newCopyVariables_it->first;
	ConstVariablePtr newV = lp->newVariable(v->getLb(), v->getUb(), Continuous);
	
	// XXX make sure this updates new_newCopyVariables
	(newCopyVariables_it->second).push_back(newV);
	
	//((new_newCopyVariables.find(v))->second).push_back(newV);
	
	// Add a constraint that says v = newV
	
	LinearFunctionPtr slf = LinearFunctionPtr(new LinearFunction());
	slf->addTerm(v,-1.0);
	slf->addTerm(newV, 1.0);
	FunctionPtr sof = (FunctionPtr) new Function(slf);
	ConstraintPtr soc = lp->newConstraint(sof, 0.0, 0.0);
      }
    }
  }
}

void relaxationInfo::linearizeQuadraticRltf(QuadraticFunctionPtr q, LinearFunctionPtr &l, 
					    LPRelaxationPtr &lp, Const Double *x) {
  // Quadratic part gets a new variable for every term
  if(q) {
    for(VariablePairGroupConstIterator it = q->begin(); it != q->end(); ++it) {
      
      ConstVariablePtr x1 = oVars.find(it->first.first)->second;
      ConstVariablePtr x2 = oVars.find(it->first.second)->second;
      ConstVariablePtr x_1;

      // Check to see if 'it' is a square term
      if(x1->getId() == x2->getId()) {
	newCopyVariables_it = new_newCopyVariables.find(x1);
	x_1 = ((new_newCopyVariables.find(x1))->second)[0];
	
	// look to see if x_1^2 already exists...
	std::map <ConstVariablePtr, ConstVariablePair>::iterator pos ;
	ConstVariablePtr y;
	ConstVariablePair Z;
	if (new_sqterms.find(x1) == new_sqterms.end()) {
	  
	  // Make a copy of the variable
	  double lb = 0.0;
	  double ub = 0.0;
	  BoundsOnProduct(x1,x_1,lb,ub);
	  y = lp->newVariable(lb, ub, Continuous);
	  new_blterms.insert(make_pair(y,ConstVariablePair(x1,x_1)));
	  new_rev_blterms.insert(make_pair(ConstVariablePair(x1,x_1), y));
	  add_blterms.insert(make_pair(y,ConstVariablePair(x1,x_1)));
	  add_rev_blterms.insert(make_pair(ConstVariablePair(x1,x_1), y));
	  
	  new_sqterms.insert(make_pair(x1,ConstVariablePair(y,x_1)));
	  new_rev_sqterms.insert(make_pair(ConstVariablePair(y,x_1), x1));
	  add_sqterms.insert(make_pair(x1,ConstVariablePair(y,x_1)));
	  add_rev_sqterms.insert(make_pair(ConstVariablePair(y,x_1), x1));
	  
	  // Secant cut
	  double x1_val = x[x1->getId()];
	  LinearFunctionPtr sCutlf = LinearFunctionPtr(new LinearFunction());
	  sCutlf->addTerm(x1, 2*x1_val);
	  sCutlf->addTerm(y, -1);
	  FunctionPtr sCutf = (FunctionPtr) new Function(sCutlf);
	  ConstraintPtr sCutc = lp->newConstraint(sCutf, -INFINITY, 
	  1 * pow(x1_val, 2)); 
	}
	else {
	  pos = new_sqterms.find(x1);
	  y = pos->second.first;
	}
	
	l->addTerm(y, it->second);
	
	// XXX seems like these two lines are un-necessary
	// Add this term to the list of square terms
	//sqterms.insert(make_pair(y,ConstVariablePair(x1,x_1)));
	//rev_sqterms.insert(make_pair(ConstVariablePair(x1,x_1), y));
      }
      else {
	//Bounds on product depend on whether variable bounds are < 0, > 0
	double lb = 0.0;
	double ub = 0.0;
	BoundsOnProduct(x1,x2,lb,ub);
	
	// look to see if w var already exists...
	std::map <ConstVariablePair, ConstVariablePtr>::iterator pos ;
	ConstVariablePtr w;
	if (new_rev_blterms.find(ConstVariablePair(x1,x2)) == new_rev_blterms.end()) {
	  w = lp->newVariable(lb, ub, Continuous);
	  
	  new_blterms.insert(make_pair(w,ConstVariablePair(x1,x2)));
	  new_rev_blterms.insert(make_pair(ConstVariablePair(x1,x2), w));
	  add_blterms.insert(make_pair(w,ConstVariablePair(x1,x2)));
	  add_rev_blterms.insert(make_pair(ConstVariablePair(x1,x2), w));
	}
	else {
	  pos = new_rev_blterms.find(ConstVariablePair(x1,x2));
	  w = pos->second;
	}
	
	l->addTerm(w,it->second);
      }
    }
  }
}

void relaxationInfo::linearizePolynomialRltf(PolyFunPtr p, LinearFunctionPtr &l, LPRelaxationPtr &lp) {
  if(p) {
    for(MonomialConstIter mit = p->termsBegin(); mit != p->termsEnd(); ++mit) {
      const VarIntMap* mit_terms = (*mit)->getTerms();
      double mit_coeff = (*mit)->getCoeff();
      monomial_terms_it = new_monomial_terms.find(*mit_terms);
      if(monomial_terms_it == new_monomial_terms.end()) {
	std::vector<ConstVariablePtr> mit_ml;
	for(VarIntMapConstIterator tit = (*mit)->termsBegin(); tit != (*mit)->termsEnd(); ++tit) {
	  ConstVariablePtr relax_tit_var = (oVars.find(tit->first))->second;
	  if(tit->second == 1) {
	    mit_ml.push_back(relax_tit_var);
	  }
	  if(tit->second >= 2) {
	    mit_ml.push_back(relax_tit_var);
	    newCopyVariables_it = new_newCopyVariables.find(relax_tit_var);
	    //	    (newCopyVariables_it->first)->write(std::cout);
	    for(int i=2; i <= tit->second; i++) {
	      //	      printf("   ");
	      //	      (newCopyVariables_it->second)[i-2]->write(std::cout);
	      mit_ml.push_back((newCopyVariables_it->second)[i-2]);
	    }
	  }
	}
	
	new_mlterms.insert(std::pair<ConstVariablePtr, std::vector<ConstVariablePtr> > (monomial_terms_it->second, mit_ml));
	new_rev_mlterms.insert(std::pair<std::vector<ConstVariablePtr>, ConstVariablePtr > (mit_ml, monomial_terms_it->second));
	add_mlterms.insert(std::pair<ConstVariablePtr, std::vector<ConstVariablePtr> > (monomial_terms_it->second, mit_ml));
	add_rev_mlterms.insert(std::pair<std::vector<ConstVariablePtr>, ConstVariablePtr > (mit_ml, monomial_terms_it->second));
      }
      bool termAdded = 0;
      for(VariableGroupConstIterator lf_it = l->termsBegin(); lf_it != l->termsEnd(); ++lf_it) {
	if(lf_it->first == monomial_terms_it->second) {
	  l->incTerm(monomial_terms_it->second, mit_coeff);
	  termAdded = 1;
	  break;
	}
      }
      if(!termAdded) {
	l->addTerm(monomial_terms_it->second, mit_coeff);
      }
    }
  }
}

void relaxationInfo::buildAdditionalTermsByGroups(LPRelaxationPtr &lp) {
  // - First bilinear terms
  for(std::map<ConstVariablePtr, ConstVariablePair>::iterator iter = add_blterms.begin();
      iter != add_blterms.end(); ++iter) {
    bool term_needs_regroup = 1;
    ConstVariablePtr x1 = iter->second.first;
    ConstVariablePtr x2 = iter->second.second;
    
    bool x1InG = 0;
    bool x2InG = 0;
    int x1Pos;
    int x2Pos;
    int cntr = 0;
    for(int it=0; it < groups.size(); it++) {
      cntr = 0;
      x1InG = 0;
      x2InG = 0;
      for(std::vector<ConstVariablePtr>::iterator iter1 = groups[it].begin();
	  iter1 != groups[it].end(); ++iter1) {
	
	if(x1->getId() == (*iter1)->getId()) {
	  x1InG = 1;
	  x1Pos = cntr;
	}
	if(x2->getId() == (*iter1)->getId()) {
	  x2InG = 1;
	  x2Pos = cntr;
	}
	cntr++;
      }
      if(x1InG && x2InG) {
	// Get all the extreme points
	std::vector<double> lb;
	std::vector<double> ub;
	for (std::vector<ConstVariablePtr>::iterator it1 = groups[it].begin(); it1 != groups[it].end(); ++it1) {
	  ConstVariablePtr v = *it1;
	  lb.push_back(v->getLb());
	  ub.push_back(v->getUb());
	}
	
	int numVars = groups[it].size();
	std::vector<int> S(numVars);
	for(int i = 0; i < numVars; i++)
	  S[i] = i;
	std::vector<std::vector<double> > V;
	allExtreme(S, lb, ub, V);
	
	LinearFunctionPtr var_lf = LinearFunctionPtr(new LinearFunction());
	var_lf->addTerm(iter->first, -1.0);
	
	for(int i=0; i<V.size(); i++) {
	  var_lf->addTerm(all_lambdas[it][i], V[i][x1Pos]*V[i][x2Pos]);
	}
	FunctionPtr f = (FunctionPtr) new Function(var_lf);
	ConstraintPtr c = lp-> newConstraint(f, 0.0, 0.0);
	term_needs_regroup = 0;
      }
    }
    if(term_needs_regroup) {
      ConstVariablePtr xtemp = (*iter).first;
      //      xtemp->write(std::cout);
      if(blterms_tobe_regrouped.find(xtemp) == blterms_tobe_regrouped.end()) {
	blterms_tobe_regrouped.insert(*iter);
      }
    }
  }
  
  add_blterms.clear();
  add_rev_blterms.clear();
  
  // -Next the multilinear terms		  
  // - Add the product constraints
  for(std::map <ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator iter = add_mlterms.begin();
      iter != add_mlterms.end(); ++iter) {
    
    bool term_needs_regroup = 1;
    bool term_in_group = 1;
    std::vector<int> pos;
    int term_member_cntr = 0;
    // Check to see if the multilinear term is in the group
    for(int it=0; it < groups.size(); it++) {
      // - Iterate over variables of the term
      for(std::vector<ConstVariablePtr>::iterator term_it = iter->second.begin(); 
	  term_it != iter->second.end(); ++term_it) {
	term_member_cntr++;
	// - Iterate over members of the group
	int cntr = 0;
	for(std::vector<ConstVariablePtr>::iterator group_member_it = groups[it].begin();
	    group_member_it != groups[it].end(); ++group_member_it) {
	  if((*term_it)->getId() == (*group_member_it)->getId()) {
	    pos.push_back(cntr);
	  }
	  cntr++;
	}
	if(pos.size() < term_member_cntr) {
	  term_in_group = 0;
	  break;
	}
      }
      
      if(term_in_group) {
	// Get all the extreme points
	std::vector<double> lb;
	std::vector<double> ub;
	for (std::vector<ConstVariablePtr>::iterator it1 = groups[it].begin(); it1 != groups[it].end(); ++it1) {
	  ConstVariablePtr v = *it1;
	  lb.push_back(v->getLb());
	  ub.push_back(v->getUb());
	}
	
	int numVars = groups[it].size();
	std::vector<int> S(numVars);
	for(int i = 0; i < numVars; i++)
	  S[i] = i;
	std::vector<std::vector<double> > V;
	allExtreme(S, lb, ub, V);
	
	LinearFunctionPtr var_lf = LinearFunctionPtr(new LinearFunction());
	var_lf->addTerm(iter->first, -1.0);
	for(int i=0; i<V.size(); i++) {
	  double lf_term_coeff = 1;
	  for(int j=0; j<pos.size(); j++) {
	    lf_term_coeff *= V[i][pos[j]];
	  }
	  var_lf->addTerm(all_lambdas[it][i], lf_term_coeff);
	}
	FunctionPtr f = (FunctionPtr) new Function(var_lf);
	ConstraintPtr c = lp-> newConstraint(f, 0.0, 0.0);
	
	term_needs_regroup = 0;
      }
    }
    if(term_needs_regroup) {
      mlterms_tobe_regrouped.insert(*iter);
    }
  }
  
  add_mlterms.clear();
  add_rev_mlterms.clear();
}

void relaxationInfo::assignNewMapsToOldMaps() {
  // all_lambdas = new_all_lambdas;
  all_lambdas.clear();
  for(vector<vector<ConstVariablePtr> >::iterator it = new_all_lambdas.begin();
      it != new_all_lambdas.end(); ++it) {
    vector<ConstVariablePtr> lambda_variables;
    for(int it1=0; it1<(*it).size(); it1++) {
      lambda_variables.push_back((*it)[it1]);
    }
    all_lambdas.push_back(lambda_variables);
  }
  
  // new_copyVariables = new_newCopyVariables
  newCopyVariables.clear();
  for(map<ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator it = new_newCopyVariables.begin();
      it != new_newCopyVariables.end(); ++it) {
    newCopyVariables.insert(*it);
  }
  
  // sqterms = new_sqterms
  for(map <ConstVariablePtr, ConstVariablePair>::iterator it = add_sqterms.begin();
      it != add_sqterms.end(); ++it) {
    //		  ConstVariablePtr x = it->first;
    //		  ConstVariablePair x_pair = it->second;
    sqterms.insert(*it);
  }
  
  // rev_sqterms = new_rev_sqterms
  for(map <ConstVariablePair, ConstVariablePtr>::iterator it = add_rev_sqterms.begin();
      it != add_rev_sqterms.end(); ++it) {
    rev_sqterms.insert(*it);
  }
  
  // blterms = new_blterms
  for(map <ConstVariablePtr, ConstVariablePair>::iterator it = add_blterms.begin();
      it != add_blterms.end(); ++it) {
    blterms.insert(*it);
  }
  
  // rev_blterms = new_rev_blterms
  for(map <ConstVariablePair, ConstVariablePtr>::iterator it = add_rev_blterms.begin();
      it != add_rev_blterms.end(); ++it) {
    rev_blterms.insert(*it);
  }
  
  // monomial_terms = new_monomial_terms
  for(map <VarIntMap, ConstVariablePtr>::iterator it = add_monomial_terms.begin();
      it != add_monomial_terms.end(); ++it) {
    monomial_terms.insert(*it);
  }
  
  // mlterms = new_mlterms
  for(map <ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator it=add_mlterms.begin();
      it != add_mlterms.end(); ++it) {
	      mlterms.insert(*it);
  }
  
  // rev_mlterms = new_rev_mlterms
  for(map <std::vector<ConstVariablePtr>,ConstVariablePtr >::iterator it=add_rev_mlterms.begin();
      it != add_rev_mlterms.end(); ++it) {
    rev_mlterms.insert(*it);
  }
  
  // max_pow = new_max_pow
  max_pow.clear();
  for(map <ConstVariablePtr, int>::iterator it = new_max_pow.begin();
      it != new_max_pow.end(); ++it) {
    max_pow.insert(*it);
  }  
}

void relaxationInfo::assignOldMapsToNewMaps() {
  // new_all_lambdas = all_lambdas;
  new_all_lambdas.clear();
  for(vector<vector<ConstVariablePtr> >::iterator it = all_lambdas.begin();
      it != all_lambdas.end(); ++it) {
    vector<ConstVariablePtr> lambda_variables;
    for(int it1=0; it1<(*it).size(); it1++) {
      lambda_variables.push_back((*it)[it1]);
    }
    new_all_lambdas.push_back(lambda_variables);
  }
  
  // new_newCopyVariables = newCopyVariables
  new_newCopyVariables.clear();
  for(map<ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator it = newCopyVariables.begin();
      it != newCopyVariables.end(); ++it) {
    new_newCopyVariables.insert(*it);
  }
  
  // new_sqterms = sqterms
  new_sqterms.clear();
  for(map <ConstVariablePtr, ConstVariablePair>::iterator it = sqterms.begin();
      it != sqterms.end(); ++it) {
    new_sqterms.insert(*it);
  }
  
  // new_rev_sqterms = rev_sqterms
  new_rev_sqterms.clear();
  for(map <ConstVariablePair, ConstVariablePtr>::iterator it = rev_sqterms.begin();
      it != rev_sqterms.end(); ++it) {
    new_rev_sqterms.insert(*it);
  }
  
  // new_blterms = blterms
  new_blterms.clear();
  for(map <ConstVariablePtr, ConstVariablePair>::iterator it = blterms.begin();
      it != blterms.end(); ++it) {
    new_blterms.insert(*it);
  }
  
  // new_rev_blterms = rev_blterms
  new_rev_blterms.clear();
  for(map <ConstVariablePair, ConstVariablePtr>::iterator it = rev_blterms.begin();
      it != rev_blterms.end(); ++it) {
    new_rev_blterms.insert(*it);
  }
  
  // monomial_terms = new_monomial_terms
  new_monomial_terms.clear();
  for(map <VarIntMap, ConstVariablePtr>::iterator it = monomial_terms.begin();
      it != monomial_terms.end(); ++it) {
    new_monomial_terms.insert(*it);
  }
  
  // new_mlterms = mlterms
  new_mlterms.clear();
  for(map <ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator it=mlterms.begin();
      it != mlterms.end(); ++it) {
    new_mlterms.insert(*it);
  }
  
  // new_rev_mlterms = rev_mlterms
  new_rev_mlterms.clear();
  for(map <std::vector<ConstVariablePtr>,ConstVariablePtr >::iterator it=rev_mlterms.begin();
      it != rev_mlterms.end(); ++it) {
    new_rev_mlterms.insert(*it);
  }
  
  // new_max_pow = max_pow
  new_max_pow.clear();
  for(map <ConstVariablePtr, int>::iterator it = max_pow.begin();
      it != max_pow.end(); ++it) {
    new_max_pow.insert(*it);
  }  
}

void makeBinaryVariablesPowers1(FunctionPtr &rltf, PolyFunPtr &p, QuadraticFunctionPtr &q, LinearFunctionPtr &l) {
  // In each term, if a binary variable takes a power bigger than 1
  // make that power 1
  // - first look at the quadratic part
  bool no_rltf_l = 0;
  if(!l) {
    no_rltf_l = 1;
    l = LinearFunctionPtr (new LinearFunction);
  }
  
  if(q) {
    for(VariablePairGroupConstIterator qit = q->begin(); qit != q->end(); ++qit) {
      ConstVariablePtr x1 = qit->first.first;
      ConstVariablePtr x2 = qit->first.second;
      if(x1 == x2 && (x1->getType() == Binary || (x1->getType() == Integer && x1->getUb() == 1 && x1->getLb() == 0))) {
	int qitCoef = qit->second;
	q->incTerm(qit->first, -1*qit->second);
	
	l->incTerm(qit->first.first, qitCoef);
	if(no_rltf_l) 
	  rltf->add(l);
      }
    }
  }
  
  // - now look at the polynomial part
  if(p) {
    for(MonomialConstIter mit = p->termsBegin(); mit != p->termsEnd(); ++mit) {
      for(VarIntMapConstIterator tit = (*mit)->termsBegin(); tit != (*mit)->termsEnd(); ++tit) {
	if((tit->first)->getType() == Binary || ((tit->first)->getType() == Integer && (tit->first)->getUb() == 1 && (tit->first)->getLb() == 0)) {
	  if(tit->second > 1)
	    (*mit)->multiply(1.0, tit->first, -1*(tit->second - 1));
	}
      }
    }
    
    p->removeLinear(l);
    p->removeQuadratic(q);
    if(no_rltf_l) 
      rltf->add(l);
  }  
}

FunctionPtr getRltfVarUbCons(VariablePtr var, LinearFunctionPtr ubf, FunctionPtr vf, double v_up, double v_lo, double &rltc_up, double &rltc_lo) {
  FunctionPtr rltf;
  // If v is = then skip it for now 
  if(v_up - v_lo > 1e-6) {
    
    // Break down the v function
    LinearFunctionPtr lvf = vf->getLinearFunction();
    QuadraticFunctionPtr qvf = vf->getQuadraticFunction();
    NonlinearFunctionPtr nvf = vf->getNonlinearFunction();
    PolyFunPtr pvf;
    if(nvf)
      pvf = boost::dynamic_pointer_cast <PolynomialFunction> (nvf);
    
    // Multiply parts of the two functions against eachother
    QuadraticFunctionPtr q_ubf_lvf;
    PolyFunPtr p_ubf_qvf;
    PolyFunPtr p_ubf_pvf;
    
    // Multiply the linear part of ubf to the linear part of vf
    if(ubf && lvf)
      q_ubf_lvf = ubf * lvf;
    
    // Multiply the linear part of ubf to the quadratic part of vf
    if(ubf && qvf)
      p_ubf_qvf = qvf * ubf;
    
    // Multiply the linear part of ubf to the polynomial part of vf
    if(ubf && pvf)
      p_ubf_pvf = pvf * ubf;
    
    // Now add them up
    // - Polynomial part
    PolyFunPtr p_mult = p_ubf_qvf + p_ubf_pvf;
    
    // If v has a lower bound
    if(v_lo > -1e10) {
      // Find c * ubf
      LinearFunctionPtr ubf_constant = -v_lo * ubf;
      
      // Find ub * vf
      LinearFunctionPtr lvf_constant = var->getUb() * lvf;
      QuadraticFunctionPtr qvf_constant = var->getUb() * qvf;
      PolyFunPtr pvf_constant = var->getUb() * pvf;
      
      // Sum them up
      PolyFunPtr p_rltf;
      QuadraticFunctionPtr q_rltf;
      LinearFunctionPtr l_rltf;
      
      if(p_mult || pvf_constant) {
	p_rltf = p_mult + pvf_constant;
	q_rltf = q_ubf_lvf + qvf_constant;
	l_rltf = lvf_constant + ubf_constant;
	
	NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
	rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
      }
      else {
	q_rltf = q_ubf_lvf + qvf_constant;
	l_rltf = ubf_constant + lvf_constant;
	rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
      }
      
      rltc_lo = var->getUb() * v_lo;
      rltc_up = INFINITY;
    }
    
    // If v has a upper bound
    if(v_up < 1e10) {
      // Find c * ubf
      LinearFunctionPtr ubf_constant = v_up * ubf;
      
      // Find ub * vf
      LinearFunctionPtr lvf_constant = -1 * var->getUb() * lvf;
      QuadraticFunctionPtr qvf_constant = -1 * var->getUb() * qvf;
      PolyFunPtr pvf_constant = -1 * var->getUb() * pvf;
      
      // Sum them up
      PolyFunPtr p_rltf;
      QuadraticFunctionPtr q_rltf;
      LinearFunctionPtr l_rltf;
      
      QuadraticFunctionPtr neg_q_ubf_lvf = -1 * q_ubf_lvf;
      
      if(p_mult || pvf_constant) {
	PolyFunPtr neg_p_mult = -1 * p_mult;
	p_rltf = neg_p_mult + pvf_constant;
	q_rltf = neg_q_ubf_lvf + qvf_constant;
	l_rltf = lvf_constant + ubf_constant;
	
	NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
	rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
      }
      else {
	q_rltf = neg_q_ubf_lvf + qvf_constant;
	l_rltf = ubf_constant + lvf_constant;
	rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
      }
      
      rltc_lo = -1 * var->getUb() * v_up;
      rltc_up = INFINITY;
    }
  }
  return rltf;
}

FunctionPtr getRltfVarLbCons(VariablePtr var, LinearFunctionPtr lbf, FunctionPtr vf, double v_up, double v_lo, double &rltc_up, double &rltc_lo) {
  FunctionPtr rltf;
  // If v is = then skip it for now 
  if(v_up - v_lo > 1e-6) {
    
    // Break down the v function
    LinearFunctionPtr lvf = vf->getLinearFunction();
    QuadraticFunctionPtr qvf = vf->getQuadraticFunction();
    NonlinearFunctionPtr nvf = vf->getNonlinearFunction();
    PolyFunPtr pvf;
    if(nvf)
      pvf = boost::dynamic_pointer_cast <PolynomialFunction> (nvf);
    
    // Multiply parts of the two functions against eachother
    QuadraticFunctionPtr q_lbf_lvf;
    PolyFunPtr p_lbf_qvf;
    PolyFunPtr p_lbf_pvf;
    
    // Multiply the linear part of lbf to the linear part of vf
    if(lbf && lvf)
      q_lbf_lvf = lbf * lvf;
    
    // Multiply the linear part of lbf to the quadratic part of vf
    if(lbf && qvf)
      p_lbf_qvf = qvf * lbf;
    
    // Multiply the linear part of lbf to the polynomial part of vf
    if(lbf && pvf)
      p_lbf_pvf = pvf * lbf;
    
    // Now add them up
    // - Polynomial part
    PolyFunPtr p_mult = p_lbf_qvf + p_lbf_pvf;
    
    // If v has a lower bound
    if(v_lo > -1e10) {
      // Find c * lbf
      LinearFunctionPtr lbf_constant = -v_lo * lbf;
      
      // Find lb * vf
      LinearFunctionPtr lvf_constant = -1 * var->getLb() * lvf;
      QuadraticFunctionPtr qvf_constant = -1 * var->getLb() * qvf;
      PolyFunPtr pvf_constant = -1 * var->getLb() * pvf;
      
      // Sum them up
      PolyFunPtr p_rltf;
      QuadraticFunctionPtr q_rltf;
      LinearFunctionPtr l_rltf;
      
      if(p_mult || pvf_constant) {
	p_rltf = p_mult + pvf_constant;
	q_rltf = q_lbf_lvf + qvf_constant;
	l_rltf = lvf_constant + lbf_constant;
	
	NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
	rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
      }
      else {
	q_rltf = q_lbf_lvf + qvf_constant;
	l_rltf = lbf_constant + lvf_constant;
	rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
      }
      
      rltc_lo = -1 * var->getLb() * v_lo;
      rltc_up = INFINITY;
    }
    
    // If v has a upper bound
    if(v_up < 1e10) {
      // Find c * lbf
      LinearFunctionPtr lbf_constant = v_up * lbf;
      
      // Find lb * vf
      LinearFunctionPtr lvf_constant = var->getLb() * lvf;
      QuadraticFunctionPtr qvf_constant = var->getLb() * qvf;
      PolyFunPtr pvf_constant = var->getLb() * pvf;
      
      // Sum them up
      PolyFunPtr p_rltf;
      QuadraticFunctionPtr q_rltf;
      LinearFunctionPtr l_rltf;
      
      QuadraticFunctionPtr neg_q_lbf_lvf = -1 * q_lbf_lvf;
      
      if(p_mult || pvf_constant) {
	PolyFunPtr neg_p_mult = -1 * p_mult;
	p_rltf = neg_p_mult + pvf_constant;
	q_rltf = neg_q_lbf_lvf + qvf_constant;
	l_rltf = lvf_constant + lbf_constant;
	
	NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
	rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
      }
      else {
	q_rltf = neg_q_lbf_lvf + qvf_constant;
	l_rltf = lbf_constant + lvf_constant;
	rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
      }
      
      rltc_lo = var->getLb() * v_up;
      rltc_up = INFINITY;
    }
  }
  
  // If v is a = constraint just multiply it with the variable
  else {
    
    // Break down the v function
    LinearFunctionPtr lvf = vf->getLinearFunction();
    QuadraticFunctionPtr qvf = vf->getQuadraticFunction();
    NonlinearFunctionPtr nvf = vf->getNonlinearFunction();
    PolyFunPtr pvf;
    if(nvf)
      pvf = boost::dynamic_pointer_cast <PolynomialFunction> (nvf);
    
    // XXX check to see if pvf has a linear and quadratic part
    // if so remove them
    
    // Multiply parts of the two functions against eachother
    QuadraticFunctionPtr q_lbf_lvf;
    PolyFunPtr p_lbf_qvf;
    PolyFunPtr p_lbf_pvf;
    
    // Multiply the linear part of lbf to the linear part of vf
    if(lbf && lvf)
      q_lbf_lvf = lbf * lvf;
    
    // Multiply the linear part of lbf to the quadratic part of vf
    if(lbf && qvf)
      p_lbf_qvf = qvf * lbf;
    
    // Multiply the linear part of lbf to the polynomial part of vf
    if(lbf && pvf)
      p_lbf_pvf = pvf * lbf;
    
    // Now add them up
    // - Polynomial part
    PolyFunPtr p_mult = p_lbf_qvf + p_lbf_pvf;
    
    // Find c * lbf
    LinearFunctionPtr lbf_constant = -v_lo * lbf;
    
    // Sum them up
    PolyFunPtr p_rltf;
    QuadraticFunctionPtr q_rltf;
    LinearFunctionPtr l_rltf;
    
    if(p_mult) {
      p_rltf = p_mult;
      q_rltf = q_lbf_lvf;
      l_rltf = lbf_constant;
      
      NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
      rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
    }
    else {
      q_rltf = q_lbf_lvf;
      l_rltf = lbf_constant;
      rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
    }
    
    rltc_lo = 0;
    rltc_up = 0;
  }  
  return rltf;
}

FunctionPtr getRltfConsCons(ConstraintPtr cons, FunctionPtr vf, double v_up, double v_lo, double &rltc_up, double &rltc_lo) {
  FunctionPtr cf = cons->getFunction();

  // Break down the constraint function
  LinearFunctionPtr lcf = cf->getLinearFunction();
  QuadraticFunctionPtr qcf = cf->getQuadraticFunction();
  NonlinearFunctionPtr ncf = cf->getNonlinearFunction();
  PolyFunPtr pcf;
  if(ncf)
    pcf = boost::dynamic_pointer_cast <PolynomialFunction> (ncf);
  
  // Break down the v function
  LinearFunctionPtr lvf = vf->getLinearFunction();
  QuadraticFunctionPtr qvf = vf->getQuadraticFunction();
  NonlinearFunctionPtr nvf = vf->getNonlinearFunction();
  PolyFunPtr pvf;
  if(nvf)
    pvf = boost::dynamic_pointer_cast <PolynomialFunction> (nvf);
  
  // Multiply parts of the two functions against eachother
  QuadraticFunctionPtr q_lcf_lvf;
  PolyFunPtr p_lcf_qvf;
  PolyFunPtr p_lcf_pvf;
  PolyFunPtr p_qcf_lvf;
  PolyFunPtr p_qcf_qvf;
  PolyFunPtr p_qcf_pvf;
  PolyFunPtr p_pcf_lvf;
  PolyFunPtr p_pcf_qvf;
  PolyFunPtr p_pcf_pvf;
  
  // Multiply the linear part of cf to the linear part of vf
  if(lcf && lvf)
    q_lcf_lvf = lcf * lvf;
  
  // Multiply the linear part of cf to the quadratic part of vf
  if(lcf && qvf)
    p_lcf_qvf = qvf * lcf;
  
  // Multiply the linear part of cf to the polynomial part of vf
  if(lcf && nvf)
    p_lcf_pvf = pvf * lcf;
  
  // Multiply the quadratic part of cf to the linear part of vf
  if(qcf && lvf)
    p_qcf_lvf = qcf * lvf;
  
  // Multiply the quadratic part of cf to the quadratic part of vf
  if(qcf && qvf)
    p_qcf_qvf = qcf * qvf;
  
  // Multiply the quadratic part of cf to the polynomial part of vf
  if(qcf && nvf)
    p_qcf_pvf = pvf * qcf;
  
  // Multiply the polynomial part of cf to the linear part of vf
  if(ncf && lvf)
    p_pcf_lvf = pcf * lvf;
  
  // Multiply the polynomial part of cf to the quadratic part of vf
  if(ncf && qvf)
    p_pcf_qvf = pcf * qvf;
  
  // Multiply the polynomial part of cf to the polynomial part of vf
  if(ncf && nvf)
    p_pcf_pvf = pcf * pvf;
  
  // Now add them up
  // - Polynomial part
  PolyFunPtr p_mult = p_lcf_qvf + p_lcf_pvf + p_qcf_lvf + p_qcf_qvf + 
    p_qcf_pvf + p_pcf_lvf + p_pcf_qvf + p_pcf_pvf;
  
  NonlinearFunctionPtr n_mult = boost::dynamic_pointer_cast <NonlinearFunction> (p_mult);
  LinearFunctionPtr dummy;
  
  FunctionPtr mult = (FunctionPtr) new Function(dummy, q_lcf_lvf, n_mult);
  
  FunctionPtr rltf;
  // If both the constraint and v have lower bound
  if(cons->getLb() > -1e10 && v_lo > -1e10) {
    // Find c * cf
    LinearFunctionPtr lcf_constant = -v_lo * lcf;
    QuadraticFunctionPtr qcf_constant = -v_lo * qcf;
    PolyFunPtr pcf_constant = -v_lo * pcf;
    
    // Find d * vf
    LinearFunctionPtr lvf_constant = -1 * cons->getLb() * lvf;
    QuadraticFunctionPtr qvf_constant = -1 * cons->getLb() * qvf;
    PolyFunPtr pvf_constant = -1 * cons->getLb() * pvf;
    
    // Sum them up
    PolyFunPtr p_rltf;
    QuadraticFunctionPtr q_rltf;
    LinearFunctionPtr l_rltf;
    
    if(p_mult || pcf_constant || pvf_constant) {
      p_rltf = p_mult + pcf_constant + pvf_constant;
      q_rltf = q_lcf_lvf + qvf_constant + qcf_constant;
      l_rltf = lcf_constant + lvf_constant;
      
      NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
      rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
    }
    else {
      q_rltf = q_lcf_lvf + qcf_constant + qvf_constant;
      l_rltf = lcf_constant + lvf_constant;
      rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
    }
    
    rltc_lo = -1 * cons->getLb() * v_lo;
    rltc_up = INFINITY;
  }
  
  // If constraint has a lower bound and v has an upper bound
  if(cons->getLb() > -1e10 && v_up < 1e10) {
    // Find c * cf
    LinearFunctionPtr lcf_constant = v_up * lcf;
    QuadraticFunctionPtr qcf_constant = v_up * qcf;
    PolyFunPtr pcf_constant = v_up * pcf;
    
    // Find d * vf
    LinearFunctionPtr lvf_constant = cons->getLb() * lvf;
    QuadraticFunctionPtr qvf_constant = cons->getLb() * qvf;
    PolyFunPtr pvf_constant = cons->getLb() * pvf;
		  
    // Sum them up
    PolyFunPtr p_rltf;
    QuadraticFunctionPtr q_rltf;
    LinearFunctionPtr l_rltf;
    
    QuadraticFunctionPtr neg_q_lcf_lvf = -1 * q_lcf_lvf;
    
    if(p_mult || pcf_constant || pvf_constant) {
      PolyFunPtr neg_p_mult = -1 * p_mult;
      p_rltf = neg_p_mult + pcf_constant + pvf_constant;
      q_rltf = neg_q_lcf_lvf + qvf_constant + qcf_constant;
      l_rltf = lvf_constant + lcf_constant;
      
      NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
      rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
    }
    else {
      q_rltf = neg_q_lcf_lvf + qcf_constant + qvf_constant;
      l_rltf = lcf_constant + lvf_constant;
      rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
    }
    
    rltc_lo = cons->getLb() * v_up;
    rltc_up = INFINITY;
  }
  
  // If constraint has an upper bound and v has a lower bound
  if(cons->getUb() < 1e10 && v_lo > -1e10) {
    // Find c * cf
    LinearFunctionPtr lcf_constant = v_lo * lcf;
    QuadraticFunctionPtr qcf_constant = v_lo * qcf;
    PolyFunPtr pcf_constant = v_lo * pcf;
    
    // Find d * vf
    LinearFunctionPtr lvf_constant = cons->getUb() * lvf;
    QuadraticFunctionPtr qvf_constant = cons->getUb() * qvf;
    PolyFunPtr pvf_constant = cons->getUb() * pvf;
    
    // Sum them up
    PolyFunPtr p_rltf;
    QuadraticFunctionPtr q_rltf;
    LinearFunctionPtr l_rltf;
    
    QuadraticFunctionPtr neg_q_lcf_lvf = -1 * q_lcf_lvf;
		  
    if(p_mult || pcf_constant || pvf_constant) {
      PolyFunPtr neg_p_mult = -1 * p_mult;
      p_rltf = neg_p_mult + pcf_constant + pvf_constant;
      q_rltf = neg_q_lcf_lvf + qvf_constant + qcf_constant;
      l_rltf = lcf_constant + lvf_constant;
      
      NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
      rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
    }
    else {
      q_rltf = neg_q_lcf_lvf + qcf_constant + qvf_constant;
      l_rltf = lcf_constant + lvf_constant;
      rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
    }
    
    rltc_lo = cons->getUb() * v_lo;
    rltc_up = INFINITY;
  }
  
  // If constraint has an upper bound and v has an upper bound
  if(cons->getUb() < 1e10 && v_up < 1e10) {
    // Find c * cf
    LinearFunctionPtr lcf_constant = -v_up * lcf;
    QuadraticFunctionPtr qcf_constant = -v_up * qcf;
    PolyFunPtr pcf_constant = -v_up * pcf;
    
    // Find d * vf
    LinearFunctionPtr lvf_constant = -1 * cons->getUb() * lvf;
    QuadraticFunctionPtr qvf_constant = -1 * cons->getUb() * qvf;
    PolyFunPtr pvf_constant = -1 * cons->getUb() * pvf;
    
    // Sum them up
    PolyFunPtr p_rltf;
    QuadraticFunctionPtr q_rltf;
    LinearFunctionPtr l_rltf;
    
    if(p_mult || pcf_constant || pvf_constant) {
      p_rltf = p_mult + pcf_constant + pvf_constant;
      q_rltf = q_lcf_lvf + qvf_constant + qcf_constant;
      l_rltf = lvf_constant + lcf_constant;
      
      NonlinearFunctionPtr n_rltf = boost::dynamic_pointer_cast <NonlinearFunction> (p_rltf);
      rltf = (FunctionPtr) new Function(l_rltf, q_rltf, n_rltf);
    }
    else {
      q_rltf = q_lcf_lvf + qcf_constant + qvf_constant;
      l_rltf = lcf_constant + lvf_constant;
      rltf = (FunctionPtr) new Function(l_rltf, q_rltf);
    }
    
    rltc_lo = -1 * cons->getUb() * v_up;
    rltc_up = INFINITY;
  }
  return rltf;
}


std::map <ConstVariablePair, ConstVariablePtr> buildRevBltermsTobeRegrouped(std::map <ConstVariablePtr, ConstVariablePair> Bl) {
  std::map <ConstVariablePair, ConstVariablePtr> rev_Bl; 
  for(std::map <ConstVariablePtr, ConstVariablePair>::iterator bl_iter = Bl.begin();
      bl_iter != Bl.end(); ++bl_iter) {
    ConstVariablePtr x1 = bl_iter->first;
    ConstVariablePair x_pair = bl_iter->second;
    
    rev_Bl.insert(make_pair(x_pair, x1));
  }
  return rev_Bl;
}

std::map <std::vector<ConstVariablePtr>, ConstVariablePtr> buildRevMltermsTobeRegrouped(std::map <ConstVariablePtr, std::vector<ConstVariablePtr> > Ml) {
  std::map <std::vector<ConstVariablePtr>, ConstVariablePtr > rev_Ml;
  for(std::map <ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator ml_iter = Ml.begin();
      ml_iter != Ml.end(); ++ml_iter) {
    ConstVariablePtr x = ml_iter->first;
    std::vector<ConstVariablePtr> x_vec = ml_iter->second;
    //		    for(int i=0; i<ml_iter->second.size(); i++) 
    //		      x_vec[i] = ml_iter->second[i];
    
    rev_Ml.insert(make_pair(x_vec, x));    
  }
  return rev_Ml;
}

void addLinearizationBlterms(std::map<ConstVariablePtr, ConstVariablePair> Bl, std::vector<ConstVariablePtr> group, 
			     LPRelaxationPtr &lp, std::vector<std::vector<double> > V, std::vector<VariablePtr> lambdavars) {
  for(std::map<ConstVariablePtr, ConstVariablePair>::iterator iter = Bl.begin();
      iter != Bl.end(); ++iter) {
    ConstVariablePtr x1 = iter->second.first;
    ConstVariablePtr x2 = iter->second.second;
    bool x1InG = 0;
    bool x2InG = 0;
    int x1Pos;
    int x2Pos;
    int cntr = 0;
    for(std::vector<ConstVariablePtr>::iterator iter1 = group.begin();
	iter1 != group.end(); ++iter1) {
      if(x1->getId() == (*iter1)->getId()) {
	x1InG = 1;
	x1Pos = cntr;
      }
      if(x2->getId() == (*iter1)->getId()) {
	x2InG = 1;
	x2Pos = cntr;
      }
      cntr++;
    }
    if(x1InG && x2InG) {
      LinearFunctionPtr var_lf = LinearFunctionPtr(new LinearFunction());
      var_lf->addTerm(iter->first, -1.0);
      for(int i=0; i<V.size(); i++) {
	var_lf->addTerm(lambdavars[i], V[i][x1Pos]*V[i][x2Pos]);
      }
      FunctionPtr f = (FunctionPtr) new Function(var_lf);
      ConstraintPtr c = lp-> newConstraint(f, 0.0, 0.0);
    }
  }
}

void addLinearizationMlterms(std::map <ConstVariablePtr, std::vector<ConstVariablePtr> > Ml, std::vector<ConstVariablePtr> group, 
			     LPRelaxationPtr &lp, std::vector<std::vector<double> > V, std::vector<VariablePtr> lambdavars) {
  for(std::map <ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator iter = Ml.begin();
      iter != Ml.end(); ++iter) {
    
    bool term_in_group = 1;
    std::vector<int> pos;
    int term_member_cntr = 0;
    // Check to see if the multilinear term is in the group
    // - Iterate over variables of the term
    for(std::vector<ConstVariablePtr>::iterator term_it = iter->second.begin(); term_it != iter->second.end(); ++term_it) {
      term_member_cntr++;
      // - Iterate over members of the group
      int cntr = 0;
      for(std::vector<ConstVariablePtr>::iterator group_member_it = group.begin();
	  group_member_it != group.end(); ++group_member_it) {
	if((*term_it)->getId() == (*group_member_it)->getId()) {
	  pos.push_back(cntr);
	}
	cntr++;
      }
      if(pos.size() < term_member_cntr) {
	term_in_group = 0;
	break;
      }
    }
    
    if(term_in_group) {
      LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
      lf->addTerm(iter->first, -1.0);
      for(int i=0; i<V.size(); i++) {
	double lf_term_coeff = 1;
	for(int j=0; j<pos.size(); j++) {
	  lf_term_coeff *= V[i][pos[j]];
	}
	lf->addTerm(lambdavars[i], lf_term_coeff);
      }
      FunctionPtr f = (FunctionPtr) new Function(lf);
      ConstraintPtr c = lp-> newConstraint(f, 0.0, 0.0);
    }
  }
}

bool isRltcSatisfied(LinearFunctionPtr l, double lo, double up, Const Double * sol) {
  printf("No new terms\n");
  double lf_val = 0;
  for(VariableGroupConstIterator it = l->termsBegin(); it != l->termsEnd(); ++it) {
    ConstVariablePtr v = it->first;
    lf_val += sol[v->getId()] * it->second;
  }
  if((lf_val - lo > 1e-7) && (up - lf_val > 1e-7))
    return true;
  else
    return false;
}

void usage()
{
  std::cout << "usage: ./rlt_degree_n {name-of-.nl-file} {the degree of rlt}\n";
}

void makeGroups(std::map <ConstVariablePtr, ConstVariablePair> blterms,
                               std::map <ConstVariablePair, ConstVariablePtr> rev_blterms,
                               std::map <ConstVariablePtr, std::vector<ConstVariablePtr> > mlterms,
                               std::map <std::vector<ConstVariablePtr>, ConstVariablePtr> rev_mlterms,
                               std::vector <std::vector<ConstVariablePtr> > &groups,
                               int groupStrategy)
{
  std::vector <ConstVariablePtr> graphVars;
  int cntr1 = 0;
  int gs = 3;

  // Get the variables that appear in the bilinear terms
  for(std::map<ConstVariablePtr, ConstVariablePair>::iterator it = blterms.begin();
      it != blterms.end(); ++it) {
    
    ConstVariablePtr x1 = it->second.first;
    ConstVariablePtr x2 = it->second.second;
    bool x1InTemp = 0;
    bool x2InTemp = 0;
    for(std::vector<ConstVariablePtr>::iterator it1 = graphVars.begin(); it1 != graphVars.end(); ++it1) {
      if(x1->getId() == (*it1)->getId())
        x1InTemp = 1;
      if(x2->getId() == (*it1)->getId())
        x2InTemp = 1;
    }
    if(!x1InTemp) {
      graphVars.push_back(x1);
      cntr1++;
    }
    if(!x2InTemp) {
      graphVars.push_back(x2);
      cntr1++;
    }
  }

  // Get the variables that appear in multilinear terms
  for(std::map<ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator it = mlterms.begin();
      it != mlterms.end(); ++it) {
    for(std::vector<ConstVariablePtr>::iterator mlt_it = it->second.begin(); mlt_it != it->second.end(); ++mlt_it) {
      int varInTemp = 0;
      for(std::vector<ConstVariablePtr>::iterator gv_it = graphVars.begin(); gv_it != graphVars.end(); ++gv_it) {
        if((*gv_it)->getId() == (*mlt_it)->getId()) {
          varInTemp = 1;
          break;
        }
      }
      if(!varInTemp) {
        graphVars.push_back(*mlt_it);
        cntr1++;
      }
    }
  }

  // Form a 2-D vector that has the bilinear and multilinear terms
  std::vector<std::vector<ConstVariablePtr> > terms;
  for(std::map<ConstVariablePtr, ConstVariablePair>::iterator bl_it = blterms.begin(); bl_it != blterms.end(); ++bl_it) {
    std::vector<ConstVariablePtr> temp;
    temp.push_back(bl_it->second.first);
    temp.push_back(bl_it->second.second);
    terms.push_back(temp);
  }

  for(std::map<ConstVariablePtr, std::vector<ConstVariablePtr> >::iterator ml_it = mlterms.begin(); ml_it != mlterms.end(); ++ml_it) {
    std::vector<ConstVariablePtr> temp;
    for(std::vector<ConstVariablePtr>::iterator t_it = ml_it->second.begin(); t_it != ml_it->second.end(); ++t_it) {
      temp.push_back(*t_it);
    }
    terms.push_back(temp);
  }

  std::vector<ConstVariablePtr> temp;
  int s = terms.size();
  int counter = 0;
  bool xInTemp = 0;
  for(std::vector<std::vector<ConstVariablePtr> >::iterator terms_it = terms.begin(); terms_it != terms.end(); ++terms_it) {
    for(std::vector<ConstVariablePtr>::iterator term_it = (*terms_it).begin(); term_it != (*terms_it).end(); ++term_it) {
      xInTemp = 0;
      for(std::vector<ConstVariablePtr>::iterator it = temp.begin(); it != temp.end(); ++it) {
        if((*it)->getId() == (*term_it)->getId()) {
          xInTemp = 1;
        }
      }
      if(!xInTemp) {
        temp.push_back(*term_it);
      }
    }
    counter++;

    if(s >=gs && counter>=gs) {
      groups.push_back(temp);
      temp.erase(temp.begin(),temp.end());
      counter = 0;
      s -= gs;
    }
  }
  if(!temp.empty())
    groups.push_back(temp);
}

int
main(int argc, char** argv)
{
  if (argc < 3) {
    usage();
    return -1;
  }

  time_start();

  Minotaur::EnvPtr env1 = (Minotaur::EnvPtr) new Minotaur::Environment();
  MINOTAUR_AMPL::AMPLInterfacePtr iface = new MINOTAUR_AMPL::AMPLInterface(env1);
  ProblemPtr inst, newInst;
  inst = iface->readInstance(argv[1]);
  newInst = iface->readInstance(argv[1]);
  int rltDegree = atoi(argv[2]);

  std::vector<FunctionPtr> v;
  std::vector<FunctionPtr> temp_v;
  std::vector<double> v_up;
  std::vector<double> v_lo;
  std::vector<double> temp_v_up;
  std::vector<double> temp_v_lo;

  for(ConstraintConstIterator con_it = inst->consBegin(); con_it != inst->consEnd(); ++con_it) {
    FunctionType con_typ = (*con_it)->getFunctionType();
    if(con_typ == Linear || con_typ == Quadratic || con_typ == Bilinear || con_typ == Polynomial) {
      FunctionPtr cf = (*con_it)->getFunction();
      v.push_back(cf);
      v_up.push_back((*con_it)->getUb());
      v_lo.push_back((*con_it)->getLb());
    }
  }

  //*********************************************************
  //*********************************************************
  // Build the relaxation for the problem
  //*********************************************************
  //*********************************************************
  int add_to_name = 3;

  LPRelaxationPtr lpr = LPRelaxationPtr(new LPRelaxation());
  LPRelaxationPtr new_lpr = LPRelaxationPtr(new LPRelaxation());
  MultilinearHandlerPtr mhandler = MultilinearHandlerPtr(new MultilinearHandler(newInst));
  MultilinearHandlerPtr new_mhandler = MultilinearHandlerPtr(new MultilinearHandler(newInst));

  bool isFeasible;
  int nCons = newInst->getNumCons();
  int nVars = newInst->getNumVars();
  vector<bool> c_list(nCons);

  relaxationInfo *info = new relaxationInfo;
  
  mhandler->relax(lpr, isFeasible, c_list);
  //  new_mhandler->relax(new_lpr, isFeasible, c_list);
  VariableConstIterator lpr_var_begin = lpr->varsBegin();
  VariableConstIterator lpr_var_end = lpr->varsEnd();

  new_lpr->newVariables(lpr_var_begin, lpr_var_end);

  for(ConstraintConstIterator con_it = lpr->consBegin(); con_it != lpr->consEnd(); ++con_it) {
    ConstraintPtr new_lpr_cons = new_lpr->newConstraint((*con_it)->getFunction(), 
							(*con_it)->getLb(), (*con_it)->getUb());
  }
  ObjectivePtr new_lpr_obj = new_lpr->newObjective((lpr->getObjective())->getFunction(), 
						   0.0, (lpr->getObjective())->getObjectiveType());
  
  info->groups = mhandler->getGroups();
  info->all_lambdas = mhandler->getAllLambdas();
  info->oVars = mhandler->getOriginalVariablesMap();
  info->rev_oVars = mhandler->getRevOriginalVariablesMap();
  info->newCopyVariables = mhandler->getNewCopyVariables();
  info->sqterms = mhandler->getSqterms();
  info->rev_sqterms = mhandler->getRevSqterms();
  info->blterms = mhandler->getBlterms();
  info->rev_blterms = mhandler->getRevBlterms();
  info->monomial_terms = mhandler->getMonomialterms();
  info->max_pow = mhandler->getMaxPow();
  info->mlterms = mhandler->getMlterms();
  info->rev_mlterms = mhandler->getRevMlterms();

  // new_groups = groups
  for(int it=0; it < (info->groups).size(); it++) {
    std::vector<ConstVariablePtr> additional_group;
    for (std::vector<ConstVariablePtr>::iterator it1 = (info->groups)[it].begin(); 
	 it1 != (info->groups)[it].end(); ++it1) {
      additional_group.push_back(*it1);
    }
    (info->new_groups).push_back(additional_group);
  }

  info->assignOldMapsToNewMaps();


  Minotaur::EnvPtr env = (Minotaur::EnvPtr) new Minotaur::Environment();
  OsiLPEnginePtr lpE = OsiLPEnginePtr(new OsiLPEngine(env));
  OsiLPEnginePtr new_lpE;
  int nv = lpr->getNumVars();
  
  lpE->load(lpr);
  lpE->solve();
  double sol_val = lpE->getSolutionValue();
  printf("Initial Solution: %f\n", sol_val);
  string status = lpE->getStatusString();
  ConstSolutionPtr lpSol = lpE->getSolution();
  const Double * solution;
  solution = lpSol->getPrimal();

  //*********************************************************
  //*********************************************************
  // Up to this point we build the relaxation of the problem
  // Next we do the iterative RLT
  // - Take two constraints and multiply them
  // - See if its linearization should be added to the LP
  //*********************************************************
  //*********************************************************

  double rltc_lo = 0;
  double rltc_up = 0;

  int round = 1;

  while(round < rltDegree) {
    //PIPI
    printf("round = %d\n", round);

    // we use these two integers to make sure that for the first
    // round the same constraint doesn't get built twice
    int con_cntr = -1;
    int v_cntr = -1;


    //****************************************************************
    //****************************************************************
    // multiply variable bound constraints against regular constraints
    //****************************************************************
    //****************************************************************
   
    for(VariableConstIterator var_it = inst->varsBegin(); var_it != inst->varsEnd(); ++var_it) {
      v_cntr = -1;

      // If the variable has a lower bound
      if((*var_it)->getLb() > -1e-10) {
	LinearFunctionPtr lbf = LinearFunctionPtr(new LinearFunction());
	lbf->addTerm(*var_it, 1);
	FunctionPtr rltf;
	
	// Iterate through the functions in v
	for(std::vector<FunctionPtr>::iterator vr_it = v.begin(); vr_it != v.end(); ++vr_it) {
	  v_cntr++;
	  FunctionPtr vf = *vr_it;

	  rltf = getRltfVarLbCons(*var_it, lbf, vf, v_up[v_cntr], v_lo[v_cntr], rltc_up, rltc_lo);

	  temp_v.push_back(rltf);
	  temp_v_lo.push_back(rltc_lo);
	  temp_v_up.push_back(rltc_up);	      


	  // Break down the rltf function
	  LinearFunctionPtr rltf_l = rltf->getLinearFunction();
	  QuadraticFunctionPtr rltf_q = rltf->getQuadraticFunction();
	  NonlinearFunctionPtr rltf_n = rltf->getNonlinearFunction();
	  PolyFunPtr rltf_p;
	  if(rltf_n)
	    rltf_p = boost::dynamic_pointer_cast <PolynomialFunction> (rltf_n);
	  
	  makeBinaryVariablesPowers1(rltf, rltf_p, rltf_q, rltf_l);
	  
	  info->updateNewMaxPow(rltf_q, rltf_p, new_lpr);
	  info->updateNewNewCopyVariables(new_lpr);
	  
	  //************************************
	  // Now linearize the function rltf
	  //************************************
	  LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
	  // Linear part of constraint remains the same
	  if (rltf_l){
	    for(VariableGroupConstIterator it = rltf_l->termsBegin(); it != rltf_l->termsEnd(); ++it) {
	      ConstVariablePtr tempVar;
	      tempVar = it->first;
	      lf->addTerm((info->oVars).find(tempVar)->second, it->second);
	    }
	  }
	  
	  info->linearizeQuadraticRltf(rltf_q, lf, new_lpr, solution);
	  info->linearizePolynomialRltf(rltf_p, lf, new_lpr);
	  
	  // Check if new terms are introduced and if not go to the next function in v
	  if(info->add_blterms.size() + info->add_mlterms.size() == 0) {
	    if(isRltcSatisfied(lf, rltc_lo, rltc_up, solution))
	      break;
	  }

	  FunctionPtr of = (FunctionPtr) new Function(lf);
	  ConstraintPtr oc = new_lpr->newConstraint(of,rltc_lo, rltc_up);
		
	  info->buildAdditionalTermsByGroups(new_lpr);
	  
	  info->rev_blterms_tobe_regrouped = buildRevBltermsTobeRegrouped(info->blterms_tobe_regrouped);
	  info->rev_mlterms_tobe_regrouped = buildRevMltermsTobeRegrouped(info->mlterms_tobe_regrouped);
	  
	  // Now make groups for the variables appearing in the terms
	  // that are not covered by any of the old groups
	  // namely variables appearing in blterms_tobe_regrouped
	  // and mlterms_tobe_regrouped
	  
	  // - Call the module that groups the variables
	  makeGroups(info->blterms_tobe_regrouped, info->rev_blterms_tobe_regrouped, info->mlterms_tobe_regrouped, 
		     info->rev_mlterms_tobe_regrouped, info->add_groups, mhandler->getGroupStrategy());
	  
	  // Now we go through the groups and for each group we find the extreme points
	  for(int it=0; it < info->add_groups.size(); it++) {
	    std::vector<double> lb;
	    std::vector<double> ub;
	    for (std::vector<ConstVariablePtr>::iterator it1 = info->add_groups[it].begin(); it1 != info->add_groups[it].end(); ++it1) {
	      ConstVariablePtr v = *it1;
	      lb.push_back(v->getLb());
	      ub.push_back(v->getUb());
	    }
	    
	    int numVars = (info->add_groups)[it].size();
	    std::vector<int> S(numVars);
	    for(int i = 0; i < numVars; i++)
	      S[i] = i;
	    std::vector<std::vector<double> > V;
	    allExtreme(S, lb, ub, V);
	    
	    // Now add the lambda variables
	    std::vector<VariablePtr> lambdavars;
	    for(int i = 0; i < V.size(); i++) {
	      VariablePtr lambda = new_lpr->newVariable(0.0, 1.0, Continuous);
	      lambdavars.push_back(lambda);
	    }
	    info->new_all_lambdas.push_back(lambdavars);
	    
	    addLinearizationBlterms(info->blterms_tobe_regrouped, info->add_groups[it], new_lpr, V, lambdavars);
	    addLinearizationMlterms(info->mlterms_tobe_regrouped, info->add_groups[it], new_lpr, V, lambdavars);
	    
	    // - Add the convexity constraints
	    LinearFunctionPtr convex_lf = LinearFunctionPtr(new LinearFunction());
	    for(int i=0; i<V.size(); i++)
	      convex_lf->addTerm(lambdavars[i], 1.0);
	    FunctionPtr convex_f = (FunctionPtr) new Function(convex_lf);
	    ConstraintPtr convex_c = new_lpr->newConstraint(convex_f, 1.0, 1.0);
	    
	    // - Add the constraint x_i = \sum \lambda_k c_k
	    int cntr = 0;
	    for(std::vector<ConstVariablePtr>::iterator iter1 = (info->add_groups)[it].begin();
		iter1 != (info->add_groups)[it].end(); ++iter1) {
	      LinearFunctionPtr sum_lf = LinearFunctionPtr(new LinearFunction());
	      for(int i=0; i<V.size(); i++) {
		sum_lf->addTerm(lambdavars[i], V[i][cntr]);
	      }
	      sum_lf->addTerm(*iter1, -1.0);
	      FunctionPtr sum_f = (FunctionPtr) new Function(sum_lf);
	      ConstraintPtr sum_c = new_lpr->newConstraint(sum_f, 0.0, 0.0);
	      cntr++;
	    }
	  }
	  
	  info->blterms_tobe_regrouped.clear();
	  info->rev_blterms_tobe_regrouped.clear();
	  info->mlterms_tobe_regrouped.clear();
	  info->rev_mlterms_tobe_regrouped.clear();
	  
	  Minotaur::EnvPtr new_env = (Minotaur::EnvPtr) new Minotaur::Environment();
	  new_lpE = OsiLPEnginePtr(new OsiLPEngine(new_env));
	  
	  new_lpE->load(new_lpr);
	  new_lpE->solve();
	  double new_sol_val = new_lpE->getSolutionValue();
	  double t = time_taken();

	  printf("Solution: %f     %f\n", new_sol_val, t);
	  //	  new_lpr->write(std::cout);
	  //	  lpr->write(std::cout);
	  
	  // Check whether the new rlt function was useful
	  if(new_sol_val - sol_val > 1e-6) {
	    lpSol = new_lpE->getSolution();
	    solution = lpSol->getPrimal();
	    lpr->clear();
	    lpr.reset();
	    
	    // - copy new_lpr to lpr
	    lpr = LPRelaxationPtr(new LPRelaxation());
	    VariableConstIterator new_lpr_var_begin = new_lpr->varsBegin();
	    VariableConstIterator new_lpr_var_end = new_lpr->varsEnd();
	    lpr->newVariables(new_lpr_var_begin, new_lpr_var_end);
	    
	    for(ConstraintConstIterator con_it = new_lpr->consBegin(); con_it != new_lpr->consEnd(); 
		++con_it) {
	      ConstraintPtr lpr_cons = lpr->newConstraint((*con_it)->getFunction(), 
							  (*con_it)->getLb(), (*con_it)->getUb());
	    }
	    
	    ObjectivePtr lpr_obj = lpr->newObjective((new_lpr->getObjective())->getFunction(), 
						     0.0, (new_lpr->getObjective())->getObjectiveType());
	    
	    // Add add_groups to the end of groups
	    for(int it=0; it < info->add_groups.size(); it++) {
	      std::vector<ConstVariablePtr> additional_group;
	      for (std::vector<ConstVariablePtr>::iterator it1 = info->add_groups[it].begin(); 
		   it1 != info->add_groups[it].end(); ++it1) {
		additional_group.push_back(*it1);
	      }
	      info->groups.push_back(additional_group);
	    }
	    info->add_groups.clear();

	    info->assignNewMapsToOldMaps();
	  }
	  else {
	    new_lpr->clear();
	    new_lpr.reset();
	    
	    // - copy lpr to new_lpr
	    new_lpr = LPRelaxationPtr(new LPRelaxation());
	    VariableConstIterator old_lpr_var_begin = lpr->varsBegin();
	    VariableConstIterator old_lpr_var_end = lpr->varsEnd();
	    new_lpr->newVariables(old_lpr_var_begin, old_lpr_var_end);
	    
	    for(ConstraintConstIterator con_it = lpr->consBegin(); con_it != lpr->consEnd(); ++con_it) {
	      ConstraintPtr lpr_cons = new_lpr->newConstraint((*con_it)->getFunction(), 
							      (*con_it)->getLb(), (*con_it)->getUb());
	    }
	    
	    ObjectivePtr lpr_obj = new_lpr->newObjective((lpr->getObjective())->getFunction(), 
							 0.0, (lpr->getObjective())->getObjectiveType());

	    // Clear add_groups
	    info->add_groups.clear();
	    
	    info->assignOldMapsToNewMaps();
	  }
	}
      }

      // if the variable has an upper bound
      v_cntr = -1;
      if((*var_it)->getUb() < 1e10) {
	LinearFunctionPtr ubf = LinearFunctionPtr(new LinearFunction());
	ubf->addTerm(*var_it, -1);
	FunctionPtr rltf;

	// Iterate through the functions in v
	for(std::vector<FunctionPtr>::iterator vr_it = v.begin(); vr_it != v.end(); ++vr_it) {
	  v_cntr++;
	  FunctionPtr vf = *vr_it;

	  rltf = getRltfVarUbCons(*var_it, ubf, vf, v_up[v_cntr], v_lo[v_cntr], rltc_up, rltc_lo);
	  
	  temp_v.push_back(rltf);
	  temp_v_lo.push_back(rltc_lo);
	  temp_v_up.push_back(rltc_up);	      

	  // Break down the rltf function
	  LinearFunctionPtr rltf_l = rltf->getLinearFunction();
	  QuadraticFunctionPtr rltf_q = rltf->getQuadraticFunction();
	  NonlinearFunctionPtr rltf_n = rltf->getNonlinearFunction();
	  PolyFunPtr rltf_p;
	  if(rltf_n)
	    rltf_p = boost::dynamic_pointer_cast <PolynomialFunction> (rltf_n);
	  
	  
	  makeBinaryVariablesPowers1(rltf, rltf_p, rltf_q, rltf_l);

	  info->updateNewMaxPow(rltf_q, rltf_p, new_lpr);
	  info->updateNewNewCopyVariables(new_lpr);
	  
	  //************************************
	  // Now linearize the function rltf
	  //************************************
	  LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
	  // Linear part of constraint remains the same
	  if (rltf_l){
	    for(VariableGroupConstIterator it = rltf_l->termsBegin(); it != rltf_l->termsEnd(); ++it) {
	      ConstVariablePtr tempVar;
	      tempVar = it->first;
	      lf->addTerm(info->oVars.find(tempVar)->second, it->second);
	    }
	  }
	  
	  info->linearizeQuadraticRltf(rltf_q, lf, new_lpr, solution);
	  info->linearizePolynomialRltf(rltf_p, lf, new_lpr);
	  
	  // Check if new terms are introduced and if not check to see if rltc_lo <= lf <= rltc_up is satisfied 
	  if(info->add_blterms.size() + info->add_mlterms.size() == 0) {
	    if(isRltcSatisfied(lf, rltc_lo, rltc_up, solution))
	      break;
	  }

	  FunctionPtr of = (FunctionPtr) new Function(lf);
	  ConstraintPtr oc = new_lpr->newConstraint(of,rltc_lo, rltc_up);
		
	  info->buildAdditionalTermsByGroups(new_lpr);

	  info->rev_blterms_tobe_regrouped = buildRevBltermsTobeRegrouped(info->blterms_tobe_regrouped);
	  info->rev_mlterms_tobe_regrouped = buildRevMltermsTobeRegrouped(info->mlterms_tobe_regrouped);

	  
	  // Now make groups for the variables appearing in the terms
	  // that are not covered by any of the old groups
	  // namely variables appearing in blterms_tobe_regrouped
	  // and mlterms_tobe_regrouped
	  
	  // - Call the module that groups the variables
	  makeGroups(info->blterms_tobe_regrouped, info->rev_blterms_tobe_regrouped, info->mlterms_tobe_regrouped, 
		     info->rev_mlterms_tobe_regrouped, info->add_groups, mhandler->getGroupStrategy());
	  
	  // Now we go through the groups and for each group we find the extreme points
	  for(int it=0; it < (info->add_groups).size(); it++) {
	    std::vector<double> lb;
	    std::vector<double> ub;
	    for (std::vector<ConstVariablePtr>::iterator it1 = (info->add_groups)[it].begin(); it1 != (info->add_groups)[it].end(); ++it1) {
	      ConstVariablePtr v = *it1;
	      lb.push_back(v->getLb());
	      ub.push_back(v->getUb());
	    }
	    
	    int numVars = (info->add_groups)[it].size();
	    std::vector<int> S(numVars);
	    for(int i = 0; i < numVars; i++)
	      S[i] = i;
	    std::vector<std::vector<double> > V;
	    allExtreme(S, lb, ub, V);
	    
	    // Now add the lambda variables
	    std::vector<VariablePtr> lambdavars;
	    for(int i = 0; i < V.size(); i++) {
	      VariablePtr lambda = new_lpr->newVariable(0.0, 1.0, Continuous);
	      lambdavars.push_back(lambda);
	    }
	    info->new_all_lambdas.push_back(lambdavars);
	    
	    addLinearizationBlterms(info->blterms_tobe_regrouped, info->add_groups[it], new_lpr, V, lambdavars);
	    addLinearizationMlterms(info->mlterms_tobe_regrouped, info->add_groups[it], new_lpr, V, lambdavars);
	    
	    // - Add the convexity constraints
	    LinearFunctionPtr convex_lf = LinearFunctionPtr(new LinearFunction());
	    for(int i=0; i<V.size(); i++)
	      convex_lf->addTerm(lambdavars[i], 1.0);
	    FunctionPtr convex_f = (FunctionPtr) new Function(convex_lf);
	    ConstraintPtr convex_c = new_lpr->newConstraint(convex_f, 1.0, 1.0);
	    
	    // - Add the constraint x_i = \sum \lambda_k c_k
	    int cntr = 0;
	    for(std::vector<ConstVariablePtr>::iterator iter1 = (info->add_groups)[it].begin();
		iter1 != (info->add_groups)[it].end(); ++iter1) {
	      LinearFunctionPtr sum_lf = LinearFunctionPtr(new LinearFunction());
	      for(int i=0; i<V.size(); i++) {
		sum_lf->addTerm(lambdavars[i], V[i][cntr]);
	      }
	      sum_lf->addTerm(*iter1, -1.0);
	      FunctionPtr sum_f = (FunctionPtr) new Function(sum_lf);
	      ConstraintPtr sum_c = new_lpr->newConstraint(sum_f, 0.0, 0.0);
	      cntr++;
	    }
	  }
	  
	  info->blterms_tobe_regrouped.clear();
	  info->rev_blterms_tobe_regrouped.clear();
	  info->mlterms_tobe_regrouped.clear();
	  info->rev_mlterms_tobe_regrouped.clear();
	  
	  Minotaur::EnvPtr new_env = (Minotaur::EnvPtr) new Minotaur::Environment();
	  new_lpE = OsiLPEnginePtr(new OsiLPEngine(new_env));
	  
	  new_lpE->load(new_lpr);
	  new_lpE->solve();

	  double new_sol_val = new_lpE->getSolutionValue();
	  double t = time_taken();
	  printf("Solution: %f     %f\n", new_sol_val, t);
	  //	  new_lpr->write(std::cout);
	  //	  lpr->write(std::cout);
	  
	  // Check whether the new rlt function was useful
	  if(new_sol_val - sol_val > 1e-6) {
	    lpSol = new_lpE->getSolution();
	    solution = lpSol->getPrimal();
	    lpr->clear();
	    lpr.reset();
	    
	    // - copy new_lpr to lpr
	    lpr = LPRelaxationPtr(new LPRelaxation());
	    VariableConstIterator new_lpr_var_begin = new_lpr->varsBegin();
	    VariableConstIterator new_lpr_var_end = new_lpr->varsEnd();
	    lpr->newVariables(new_lpr_var_begin, new_lpr_var_end);
	    
	    for(ConstraintConstIterator con_it = new_lpr->consBegin(); con_it != new_lpr->consEnd(); 
		++con_it) {
	      ConstraintPtr lpr_cons = lpr->newConstraint((*con_it)->getFunction(), 
							  (*con_it)->getLb(), (*con_it)->getUb());
	    }
	    
	    ObjectivePtr lpr_obj = lpr->newObjective((new_lpr->getObjective())->getFunction(), 
						     0.0, (new_lpr->getObjective())->getObjectiveType());
	    
	    // Add add_groups to the end of groups
	    for(int it=0; it < (info->add_groups).size(); it++) {
	      std::vector<ConstVariablePtr> additional_group;
	      for (std::vector<ConstVariablePtr>::iterator it1 = (info->add_groups)[it].begin(); 
		   it1 != (info->add_groups)[it].end(); ++it1) {
		additional_group.push_back(*it1);
	      }
	      (info->groups).push_back(additional_group);
	    }
	    (info->add_groups).clear();
	    
	    info->assignNewMapsToOldMaps();
	  }
	  else {
	    new_lpr->clear();
	    new_lpr.reset();
	    
	    // - copy lpr to new_lpr
	    new_lpr = LPRelaxationPtr(new LPRelaxation());
	    VariableConstIterator old_lpr_var_begin = lpr->varsBegin();
	    VariableConstIterator old_lpr_var_end = lpr->varsEnd();
	    new_lpr->newVariables(old_lpr_var_begin, old_lpr_var_end);
	    
	    for(ConstraintConstIterator con_it = lpr->consBegin(); con_it != lpr->consEnd(); ++con_it) {
	      ConstraintPtr lpr_cons = new_lpr->newConstraint((*con_it)->getFunction(), 
							      (*con_it)->getLb(), (*con_it)->getUb());
	    }
	    
	    ObjectivePtr lpr_obj = new_lpr->newObjective((lpr->getObjective())->getFunction(), 
							 0.0, (lpr->getObjective())->getObjectiveType());
	    
	    // Clear add_groups
	    (info->add_groups).clear();
	    
	    info->assignOldMapsToNewMaps();
	  }
	}
      }
    }


    // ******************************************
    // Multiply two constraints against eachother
    // ******************************************

    for(ConstraintConstIterator con_it = inst->consBegin(); con_it != inst->consEnd(); ++con_it) {
      con_cntr++;
      v_cntr = -1;
      // if the constraint is =, then just skip multiplying it with another constraint
      if((*con_it)->getUb() - (*con_it)->getLb() > 1e-6) {
	for(std::vector<FunctionPtr>::iterator vr_it = v.begin(); vr_it != v.end(); ++vr_it) {
	  v_cntr++;
	  
	  if(v_up[v_cntr] - v_lo[v_cntr] > 1e-6) {
	    
	    // if it is the first round, two constraints should be multiplied against eachother only once
	    if((round == 1 && v_cntr >= con_cntr) || round > 1) {
	      FunctionPtr cf = (*con_it)->getFunction();
	      FunctionPtr vf = *vr_it;
	      
	      FunctionType cf_typ = cf->getType();
	      FunctionType vf_typ = vf->getType();
	      if((cf_typ == Linear || cf_typ == Quadratic || cf_typ == Polynomial) && 
		 (vf_typ == Linear || vf_typ == Quadratic || vf_typ == Polynomial)) {
		
		FunctionPtr rltf;
		rltf = getRltfConsCons((*con_it), vf, v_up[v_cntr], v_lo[v_cntr], rltc_up, rltc_lo);
		  
		temp_v.push_back(rltf);
		temp_v_lo.push_back(rltc_lo);
		temp_v_up.push_back(rltc_up);	      

		// Break down the rltf function
		LinearFunctionPtr rltf_l = rltf->getLinearFunction();
		QuadraticFunctionPtr rltf_q = rltf->getQuadraticFunction();
		NonlinearFunctionPtr rltf_n = rltf->getNonlinearFunction();
		PolyFunPtr rltf_p;
		if(rltf_n)
		  rltf_p = boost::dynamic_pointer_cast <PolynomialFunction> (rltf_n);
		
		makeBinaryVariablesPowers1(rltf, rltf_p, rltf_q, rltf_l);
		
		info->updateNewMaxPow(rltf_q, rltf_p, new_lpr);
		info->updateNewNewCopyVariables(new_lpr);

		//************************************
		// Now linearize the function rltf
		//************************************
		LinearFunctionPtr lf = LinearFunctionPtr(new LinearFunction());
		// Linear part of constraint remains the same
		if (rltf_l){
		  for(VariableGroupConstIterator it = rltf_l->termsBegin(); it != rltf_l->termsEnd(); ++it) {
		    ConstVariablePtr tempVar;
		    tempVar = it->first;
		    lf->addTerm((info->oVars).find(tempVar)->second, it->second);
		  }
		}

		info->linearizeQuadraticRltf(rltf_q, lf, new_lpr, solution);
		info->linearizePolynomialRltf(rltf_p, lf, new_lpr);

		// Check if new terms are introduced and if not check to see if rltc_lo <= lf <= rltc_up is satisfied 
		if(info->add_blterms.size() + info->add_mlterms.size() == 0) {
		  if(isRltcSatisfied(lf, rltc_lo, rltc_up, solution))
		    break;
		}

		FunctionPtr of = (FunctionPtr) new Function(lf);
		ConstraintPtr oc = new_lpr->newConstraint(of,rltc_lo, rltc_up);
		
		info->buildAdditionalTermsByGroups(new_lpr);
		
		info->rev_blterms_tobe_regrouped = buildRevBltermsTobeRegrouped(info->blterms_tobe_regrouped);
		info->rev_mlterms_tobe_regrouped = buildRevMltermsTobeRegrouped(info->mlterms_tobe_regrouped);
		
		
		// Now make groups for the variables appearing in the terms
		// that are not covered by any of the old groups
		// namely variables appearing in blterms_tobe_regrouped
		// and mlterms_tobe_regrouped
		
		// - Call the module that groups the variables
		makeGroups(info->blterms_tobe_regrouped, info->rev_blterms_tobe_regrouped, info->mlterms_tobe_regrouped, 
			   info->rev_mlterms_tobe_regrouped, info->add_groups, mhandler->getGroupStrategy());
		
		// Now we go through the groups and for each group we find the extreme points
		for(int it=0; it < (info->add_groups).size(); it++) {
		  std::vector<double> lb;
		  std::vector<double> ub;
		  for (std::vector<ConstVariablePtr>::iterator it1 = (info->add_groups)[it].begin(); 
		       it1 != (info->add_groups)[it].end(); ++it1) {
		    ConstVariablePtr v = *it1;
		    lb.push_back(v->getLb());
		    ub.push_back(v->getUb());
		  }
		  
		  int numVars = (info->add_groups)[it].size();
		  std::vector<int> S(numVars);
		  for(int i = 0; i < numVars; i++)
		    S[i] = i;
		  std::vector<std::vector<double> > V;
		  allExtreme(S, lb, ub, V);
		  
		  // Now add the lambda variables
		  std::vector<VariablePtr> lambdavars;
		  for(int i = 0; i < V.size(); i++) {
		    VariablePtr lambda = new_lpr->newVariable(0.0, 1.0, Continuous);
		    lambdavars.push_back(lambda);
		  }
		  info->new_all_lambdas.push_back(lambdavars);
		  
		  addLinearizationBlterms(info->blterms_tobe_regrouped, info->add_groups[it], 
					  new_lpr, V, lambdavars);
		  addLinearizationMlterms(info->mlterms_tobe_regrouped, info->add_groups[it],
					  new_lpr, V, lambdavars);
	  
		  // - Add the convexity constraints
		  LinearFunctionPtr convex_lf = LinearFunctionPtr(new LinearFunction());
		  for(int i=0; i<V.size(); i++)
		    convex_lf->addTerm(lambdavars[i], 1.0);
		  FunctionPtr convex_f = (FunctionPtr) new Function(convex_lf);
		  ConstraintPtr convex_c = new_lpr->newConstraint(convex_f, 1.0, 1.0);
		  
		  // - Add the constraint x_i = \sum \lambda_k c_k
		  int cntr = 0;
		  for(std::vector<ConstVariablePtr>::iterator iter1 = (info->add_groups)[it].begin();
		      iter1 != (info->add_groups)[it].end(); ++iter1) {
		    LinearFunctionPtr sum_lf = LinearFunctionPtr(new LinearFunction());
		    for(int i=0; i<V.size(); i++) {
		      sum_lf->addTerm(lambdavars[i], V[i][cntr]);
		    }
		    sum_lf->addTerm(*iter1, -1.0);
		    FunctionPtr sum_f = (FunctionPtr) new Function(sum_lf);
		    ConstraintPtr sum_c = new_lpr->newConstraint(sum_f, 0.0, 0.0);
		    cntr++;
		  }
		}

		(info->blterms_tobe_regrouped).clear();
		(info->rev_blterms_tobe_regrouped).clear();
		(info->mlterms_tobe_regrouped).clear();
		(info->rev_mlterms_tobe_regrouped).clear();

		Minotaur::EnvPtr new_env = (Minotaur::EnvPtr) new Minotaur::Environment();
		new_lpE = OsiLPEnginePtr(new OsiLPEngine(new_env));
		
		new_lpE->load(new_lpr);
		new_lpE->solve();

		double new_sol_val = new_lpE->getSolutionValue();
		double t = time_taken();
		printf("Solution: %f     %f\n", new_sol_val, t);
		//		new_lpr->write(std::cout);
		//		lpr->write(std::cout);
		
		// Check whether the new rlt function was useful
		if(new_sol_val - sol_val > 1e-6) {
		  lpSol = lpE->getSolution();
		  solution =  lpSol->getPrimal();
		  lpr->clear();
		  lpr.reset();

		  // - copy new_lpr to lpr
		  lpr = LPRelaxationPtr(new LPRelaxation());
		  VariableConstIterator new_lpr_var_begin = new_lpr->varsBegin();
		  VariableConstIterator new_lpr_var_end = new_lpr->varsEnd();
		  lpr->newVariables(new_lpr_var_begin, new_lpr_var_end);
		  
		  for(ConstraintConstIterator con_it = new_lpr->consBegin(); con_it != new_lpr->consEnd(); 
		      ++con_it) {
		    ConstraintPtr lpr_cons = lpr->newConstraint((*con_it)->getFunction(), 
								(*con_it)->getLb(), (*con_it)->getUb());
		  }

		  ObjectivePtr lpr_obj = lpr->newObjective((new_lpr->getObjective())->getFunction(), 
							   0.0, (new_lpr->getObjective())->getObjectiveType());

		  // Add add_groups to the end of groups
		  for(int it=0; it < (info->add_groups).size(); it++) {
		    std::vector<ConstVariablePtr> additional_group;
		    for (std::vector<ConstVariablePtr>::iterator it1 = (info->add_groups)[it].begin(); 
			 it1 != (info->add_groups)[it].end(); ++it1) {
		      additional_group.push_back(*it1);
		    }
		    (info->groups).push_back(additional_group);
		  }
		  (info->add_groups).clear();
		
		  info->assignNewMapsToOldMaps();
		}
		else {
		  new_lpr->clear();
		  new_lpr.reset();

		  // - copy lpr to new_lpr
		  new_lpr = LPRelaxationPtr(new LPRelaxation());
		  VariableConstIterator old_lpr_var_begin = lpr->varsBegin();
		  VariableConstIterator old_lpr_var_end = lpr->varsEnd();
		  new_lpr->newVariables(old_lpr_var_begin, old_lpr_var_end);
		  
		  for(ConstraintConstIterator con_it = lpr->consBegin(); con_it != lpr->consEnd(); ++con_it) {
		    ConstraintPtr lpr_cons = new_lpr->newConstraint((*con_it)->getFunction(), 
								(*con_it)->getLb(), (*con_it)->getUb());
		  }

		  ObjectivePtr lpr_obj = new_lpr->newObjective((lpr->getObjective())->getFunction(), 
							       0.0, (lpr->getObjective())->getObjectiveType());

		  // Clear add_groups
		  (info->add_groups).clear();
		
		  info->assignOldMapsToNewMaps();
		}
	      }
	    }
	  }
	}
      }
    }

    v.clear();
    v_up.clear();
    v_lo.clear();

    for(std::vector<FunctionPtr>::iterator temp_v_it = temp_v.begin(); temp_v_it != temp_v.end(); ++temp_v_it)
      v.push_back(*temp_v_it);
    
    for(int i = 0; i < temp_v_up.size(); i++) {
      v_up.push_back(temp_v_up[i]);
      v_lo.push_back(temp_v_lo[i]);
    }
    

    // Add the new RLT constraints to the problem
    for(int i = 0; i < v.size(); i++) {
      ConstraintPtr rltCon = newInst->newConstraint(v[i], v_lo[i], v_up[i]);
    }

    // clear the temp_v vectors
    temp_v.clear();
    temp_v_up.clear();
    temp_v_lo.clear();

    round++;
  }

  cout << "After solve : ";
  cout << new_lpE->getStatusString();
  cout << " , " <<new_lpE->getStatus();
  cout << " , " << new_lpr->getNumVars();
  cout << " , " << new_lpr->getNumCons();
  cout << " , " << new_lpE->getSolutionValue() << endl;
  /*
  Const Double *sol = new_lpE->getSolution();
  /*
  for(int i=0; i<nv; i++)
    cout << " , " << sol[i];
    //    cout <<"var["<<i<<"] = " << sol[i]<<endl;
  cout << endl;
  */
  return 1;
}


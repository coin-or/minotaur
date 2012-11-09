// ===========================================================================
// $Date: 2010-04-29 11:42:32 -0500 (Thu, 29 Apr 2010) $
// @author Zhen Xie, zhenxie -at- mcs.anl.gov
// $Id: problem.cpp 634 2010-03-08 04:35:34Z xzhen $
// ===========================================================================
#include "problem.hpp"

void problem::show(ostream &os, const short stat, const size_t tag)
{
  // os << "need to be implemented: show the objective and variables" << endl;
  double vobj = si_->getObjValue(); 
  os << right << setw(4)  << tag
     << right << setw(14) << this->name()  
     << right << setw(6)  << vobj;
  if(stat == 1) 
    os << right << setw(6) << "(OPT)" << endl;
  else if(stat == 0) 
    os << right << setw(6) << "(INF)" << endl;
  else 
    os << right << setw(6) << "(ERR)" << endl;
  
  os << "------------------------" << endl;
  for(size_t i=0;i<pvar_->size();i++){
    variable *pv = (*pvar_)[i];
    os << left  << setw(18) << pv->name()  
       << right << setw(6)  << pv->value()  
       << right << setw(6)  << pv->margin()
       << endl;
  }
  for(size_t i=0;i<noc();i++){
    constraint* pc = cons_[i];
    os << left << setw(18) << pc->name()
       << right << setw(6) << pc->level()
       << right << setw(6) << pc->margin()
       << endl;
  }
  os << "------------------------" << endl;
}

void problem::addObj(const objective* const pobj)
{
  vector<size_t>   vidx; 
  vector<double>   vval;
  pobj->extract(vidx, vval);  
  size_t k = vidx.size();
  
  int    *aidx   = new int   [k]; 
  double *aval   = new double[k];
  copy(vidx.begin(), vidx.end(), aidx);
  copy(vval.begin(), vval.end(), aval);
  
  CoinPackedVector cpv(k, aidx, aval);
  si_->addRow(cpv, pobj->lb(), pobj->ub());
  
  delete [] aidx;
  delete [] aval;
}

void problem::addCon(const constraint* const pcon)
{
  vector<size_t>   vidx; 
  vector<double>   vval;
  pcon->extract(vidx, vval);  
  size_t k = vidx.size();
  
  int    *aidx   = new int   [k]; 
  double *aval   = new double[k];
  copy(vidx.begin(), vidx.end(), aidx);
  copy(vval.begin(), vval.end(), aval);
  
  CoinPackedVector cpv(k, aidx, aval);
  si_->addRow(cpv, pcon->lb(), pcon->ub());
  
  delete [] aidx;
  delete [] aval;
}

void problem::save(const size_t tag)
{
  ostringstream ost;
  ost << this->name() << "_" << setfill('0') << setw(2) << tag;
  si_->writeLp(ost.str().c_str());
}

void problem::writeMPS(const size_t tag)
{
  ostringstream ost;
  ost << this->name() << "_" << setfill('0') << setw(2) << tag;
  si_->writeMps(ost.str().c_str());
}

short problem::status()const
{
  if(si_->isProvenOptimal()){	
    return 1;			// optimal
  }else if(si_->isProvenPrimalInfeasible() && !si_->isProvenDualInfeasible()){
    return 0;			// infeasible
  }else{
    return -1;			// error
  }
}

void problem::check()
{
  // check the objective value 
  const double* rowa = si_->getRowActivity();
  const double* rowp = si_->getRowPrice();
  
  size_t shift = noo();
  
  double  dobj = 0.0;
  for(size_t i=0;i<noc();i++){
    dobj += rowp[i+shift] * rowa[i+shift];
  }
  cout << "primal objective: " << setw(6) << si_->getObjValue() << "; "
       << "duel objective: "   << setw(6) << dobj               << "; "
       << "gap: "              << setw(6) << si_->getObjValue() - dobj
       << endl;
}

void problem::update()
{
  const double* drow = si_->getRowPrice();
  const double* dvar = si_->getReducedCost();
  const double* arow = si_->getRowActivity();
  size_t shift = noo();
  
  for(size_t i=0;i<noc();i++){
    cons_[i]->margin(drow[i+shift]); 
    cons_[i]->level(arow[i+shift]); 
  }
  
  const double *solu = si_->getColSolution();
  
  for(size_t i=0;i<nov();i++){
    (*pvar_)[i]->margin(dvar[i]);
    (*pvar_)[i]->value(solu[i]);
  }
}

void problem::updateVarLb(const size_t i, const double val)
{
  si_->setColLower(i, val);
}

void problem::updateVarUb(const size_t i, const double val)
{
  si_->setColUpper(i, val);
}

void problem::updateVarFx(const size_t i)
{
  double  val = (*pvar_)[i]->value();
  si_->setColLower(i, val);
  si_->setColUpper(i, val);
}

void problem::updateVarFx(const size_t i, const double val)
{
  (*pvar_)[i]->value(val);
  si_->setColLower(i, val);
  si_->setColUpper(i, val);
}

void problem::updateConLb(const size_t i, const double val)
{
  size_t shift = noo();
  cons_[i]->lb(val);
  si_->setRowLower(shift + i, val);
}

void problem::updateConUb(const size_t i, const double val)
{
  size_t shift = noo();
  cons_[i]->ub(val);
  si_->setRowUpper(shift + i, val);
}

void problem::updateConLb(const size_t n,
			  const size_t* const loc,const double* const val)
{
  for(size_t i=0;i<n;i++) updateConLb(loc[i], val[i]);
}

void problem::updateConUb(const size_t n,
			  const size_t* const loc,const double* const val)
{
  for(size_t i=0;i<n;i++) updateConUb(loc[i], val[i]);
}

void problem::updateConLb(const size_t n, const double* const val)
{
  assert(n <= cons_.size());
  for(size_t i=0;i<n;i++) updateConLb(i,val[i]);
}

void problem::updateConUb(const size_t n, const double* const val)
{
  assert(n <= cons_.size());
  for(size_t i=0;i<n;i++) updateConUb(i,val[i]);
}

objective* problem::newObj(scoped_ptr<uuid>     &uid,
			   const short          &relation, 
			   const vector<size_t> &indx,
			   const vector<Double> &coef, 
			   const Double         &shift)
{
  objective obj(relation, pvar_, indx, coef, shift);
  objective *pp = paObj_->add(obj); pp->id(uid->next()); 
  this->add(pp);
  
  return pp;
}

objective* problem::newObj(scoped_ptr<uuid>     &uid,
			   const short          &relation, 
			   const linearfunc     &lf, 
			   const Double         &shift)
{
  objective obj(relation, lf, shift);
  objective *pp = paObj_->add(obj); pp->id(uid->next()); 
  this->add(pp);
  
  return pp;
}

constraint* problem::newCon(scoped_ptr<uuid>     &uid,
			    const linearfunc     &lf, 
			    const Double         &lb, 
			    const Double         &ub)
{
  constraint con(lf, lb, ub);
  constraint *pp = paCon_->add(con); pp->id(uid->next()); 
  this->add(pp);
  
  return pp;
}

constraint* problem::newCon(scoped_ptr<uuid>     &uid,
			    const vector<size_t> &indx,
			    const vector<Double> &coef, 
			    const Double         &lb, 
			    const Double         &ub)
{
  constraint con(pvar_, indx, coef, lb, ub);
  constraint *pp = paCon_->add(con); pp->id(uid->next()); 
  this->add(pp);
  
  return pp;
}

vector<constraint*> problem::read(const char* const fnin, scoped_ptr<uuid> &uid)
{
  vector<constraint*> vocp;	// vector of constraint pointers
  
  ifstream fin(fnin);
  string str; istringstream ins;
  
  double lb, ub;
  
  linearfunc lf(pvar_);
  lf.read(fin,false); this->newObj(uid,1,lf);
  
  while(getline(fin, str, ':')){
    // read the lower bound
    trim(str);
    if(str.empty()) lb = -INF;
    else {
      ins.str(""); ins.clear(); ins.str(str);
      ins >> lb;
    }
    // read the linear function
    getline(fin, str, ':'); trim(str);
    lf.read(str,false);
    
    // read the upper bound
    getline(fin, str); trim(str);
    if(str.empty()) ub =  INF;
    else {
      ins.str(""); ins.clear(); ins.str(str);
      ins >> ub;
    }
    constraint* pc = this->newCon(uid, lf, lb, ub);
    vocp.push_back(pc);
  }
  
  fin.close();
  return vocp;
}

void problem::solveAux()
// form, load and solve the auxiliary problem to get the row prices
// it does not take account the objective like constraints. Note
// that the original objective function is not used here.
{
  size_t ncol = nov();		// x
  size_t ncon = cons_.size();	// lb <= Ax <= ub type of constraints
  size_t nvar = ncol + 2*ncon;
  
  // every constaint get two auxilary variables
  CoinPackedMatrix *matrix =  new CoinPackedMatrix(false,0,nvar);
  
  double* var_lb   = new double[nvar]; // lower bound for all vars
  double* var_ub   = new double[nvar]; // upper bound for all vars
  
  // set the lower and upper bound for all the variables
  for(size_t i=0;i<ncol;i++){
    variable *pv = (*pvar_)[i];
    var_lb[i] = pv->lb();
    var_ub[i] = pv->ub();
  }
  
  // set the lower and upper bound for all the auxilary variables
  for(size_t i=0;i<2*ncon;i++){
    var_lb[ncol + i] = 0;
    var_ub[ncol + i] = INF;
  }
  
  // set the lower and upper for each constraints
  double *row_lb = new double[ncon];
  double *row_ub = new double[ncon];
  int    *aidx   = NULL; 
  double *aval   = NULL;
  vector<size_t>   vidx; 
  vector<double>   vval;
  
  for(size_t i=0;i<ncon;i++){
    row_lb[i] = cons_[i]->lb();
    row_ub[i] = cons_[i]->ub();
    
    cons_[i]->extract(vidx, vval); size_t k = vidx.size();
    aidx = new int   [k+2];	// extra two position for auxilary variables
    aval = new double[k+2];
    
    copy(vidx.begin(), vidx.end(), aidx);
    copy(vval.begin(), vval.end(), aval);
    
    aidx[k]   =  ncol + 2*i; 
    aval[k]   =  1.0;
    
    aidx[k+1] =  ncol + 2*i + 1;
    aval[k+1] = -1.0;
    
    CoinPackedVector cpv(k+2, aidx, aval);
    matrix->appendRow(cpv);
    
    delete [] aidx;
    delete [] aval;
  }
  
  double *obj = new double[nvar]; 
  fill(obj,      obj+ncol, 0);
  fill(obj+ncol, obj+nvar, 1);
  
  OsiSolverInterface *clp = new OsiClpSolverInterface; 
  clp->setObjSense(1);
  clp->messageHandler()->setLogLevel(0); 
  
  clp->loadProblem(*matrix, var_lb, var_ub, obj, row_lb, row_ub);
  clp->writeLp("aux");
  
  delete [] obj;
  
  delete [] var_lb;
  delete [] var_ub;
  delete [] row_lb;
  delete [] row_ub;
  
  clp->initialSolve();
  
  const double* drow = clp->getRowPrice();
  for(size_t i=0;i<ncon;i++) {
    cons_[i]->margin(drow[i]);
  }
  
  delete clp;
}

void problem::load()
{
  size_t ncol = nov();		// x
  size_t nobj = objs_.size();	// -d >=/<= -z + Ax type of constraints
  size_t ncon = cons_.size();	// lb <= Ax <= ub type of constraints
  
  CoinPackedMatrix *matrix =  new CoinPackedMatrix(false,0,ncol+1);
  
  double* var_lb   = new double[ncol+1]; // lower bound for all vars
  double* var_ub   = new double[ncol+1]; // upper bound for all vars
  
  // set the lower and upper bound for all the variables
  variable *pv  = NULL;
  for(size_t i=0;i<ncol;i++){
    pv = (*pvar_)[i];
    var_lb[i] = pv->lb();
    var_ub[i] = pv->ub();
  }
  
  // the last variable is reserved for z;
  var_lb[ncol]   = -INF;
  var_ub[ncol]   =  INF;
  
  // set the lower and upper for each constraints
  double *row_lb = new double[nobj + ncon];
  double *row_ub = new double[nobj + ncon];
  int    *aidx   = NULL; 
  double *aval   = NULL;
  vector<size_t>   vidx; 
  vector<double>   vval;
  
  for(size_t i=0; i < nobj;i++){
    row_lb[i] = objs_[i]->lb();
    row_ub[i] = objs_[i]->ub(); 
    
    objs_[i]->extract(vidx, vval);  size_t k = vidx.size();
    aidx = new int   [k];
    aval = new double[k];
    
    copy(vidx.begin(), vidx.end(), aidx);
    copy(vval.begin(), vval.end(), aval);
    
    CoinPackedVector cpv(k, aidx, aval);
    matrix->appendRow(cpv);
    
    delete [] aidx;
    delete [] aval;
  }
  
  for(size_t i=0;i<ncon;i++){
    row_lb[i+nobj] = cons_[i]->lb();
    row_ub[i+nobj] = cons_[i]->ub();
    
    cons_[i]->extract(vidx, vval); size_t k = vidx.size();
    aidx = new int   [k];
    aval = new double[k];
    
    copy(vidx.begin(), vidx.end(), aidx);
    copy(vval.begin(), vval.end(), aval);
    
    CoinPackedVector cpv(k, aidx, aval);
    matrix->appendRow(cpv);
    
    delete [] aidx;
    delete [] aval;
  }
  
  double *obj = new double[ncol+1]; 
  fill(obj, obj+ncol, 0);
  obj[ncol] = 1;
  
  si_->loadProblem(*matrix, var_lb, var_ub, obj, row_lb, row_ub);
  si_->writeLp("estt");
  delete [] obj;
  
  delete [] var_lb;
  delete [] var_ub;
  delete [] row_lb;
  delete [] row_ub;
}

linearfunc problem::cut(const short cutType,
			const variableHandler* const comPv, // common variables
			const variableHandler* const decPv, // decision variables
			double  &shift) 
{
  size_t k = comPv->size();
  
  linearfunc lf(decPv);
  if(cutType == 1){		// optimul cut
    lf = (objs_[0])->sub(k,decPv);
  }
  // cout << lf << endl;
  
  shift = 0.0;
  // contributions from variables
  for(size_t i=0;i<comPv->size(); i++){
    variable *p = (*comPv)[i];
    if(fabs(p->value()) > EPSILON && fabs(p->margin()) > EPSILON){
      shift += p->value() * p->margin();
    }
  }
  
  // m(h-By) for all constraints
  for(size_t i=0;i<cons_.size();i++){
    constraint* pc = cons_[i];
    // cout << right << setw(4) << pc->margin() << " " << pc->sub(k, decPv);
    if(pc->margin() > EPSILON){
      shift += pc->lb()          * pc->margin();
      lf    -= pc->sub(k, decPv) * pc->margin();
    }else if(pc->margin() < -EPSILON){
      shift += pc->ub()          * pc->margin();
      lf    -= pc->sub(k, decPv) * pc->margin();
    }
  }
  // cout << lf << shift << endl;
  return lf;
}


short problem::sproc(const size_t iter,
		     const variableHandler* const comVar, // common variables
		     const variableHandler* const decVar, // decision variables
		     const vector<double> &dvv, 
		     double &bound, double &shift, 
		     vector<size_t> &idx, vector<double> &val)
{
  size_t nodv = dvv.size();
  size_t nocv = pvar_->size() - nodv;
  short  stat = -1;
  
  for(size_t i=0;i<nodv;i++) {
    (*decVar)[i]->fix(dvv[i]);
    updateVarFx(nocv+i); 
  }
  
  iter == 0 ? this->initialSolve() : this->resolve();
  
  stat = this->status();
  
  if(stat == 1){
    bound = si_->getObjValue();
    linearfunc cutLf(this->cut(1, comVar, decVar, shift));
    cutLf.extract(idx, val);
  }else if(stat == 0){
    this->solveAux();
    linearfunc cutLf(this->cut(0, comVar, decVar, shift));
    cutLf.extract(idx, val);
  }
  
  
  return stat;
}

short problem::mproc(size_t &iter,
		     scoped_ptr<uuid> &uid, 
		     const short cutType,
		     vector<double> &dvv, 
		     double &bound, const double &shift, 
		     const vector<size_t> &idx, const vector<double> &val)
{
  objective*  ocut = NULL;
  constraint* icut = NULL;
  
  if(cutType == 1){
    ocut  = this->newObj(uid, 1, idx, val, shift);
    if(iter > 0) this->addObj(ocut);
  }else{
    icut  = this->newCon(uid, idx, val, -INF, -shift);
    if(iter > 0) this->addCon(icut);
  }
  
  if(iter == 0){
    this->load();
    this->initialSolve();
  }else{
    this->resolve();
  }
  
  size_t nodv = dvv.size();
  size_t nocv = pvar_->size() - nodv;
  
  short stat = this->status(); 
  if(stat == 1){
    bound = si_->getObjValue();
    for(size_t i=0;i<nodv;i++){
      variable *pv = (*pvar_)[nocv + i];
      dvv[i] = pv->value();
    }
  }
  
  return stat;
}

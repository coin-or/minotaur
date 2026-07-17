////
//     Minotaur -- It's only 1/2 bull
//
//     (C)opyright 2010 - 2025 The Minotaur Team.
//

/**
 * \file PowHandler.cpp
 * \brief Implement the handler for functions of type y = x^k.
 * \author Neeraj Kumar, IIT Bombay
 */

#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "MinotaurConfig.h"
#include "Branch.h"
#include "BrVarCand.h"
#include "Constraint.h"
#include <cstdint>
#include "Environment.h"
#include "PowHandler.h"
#include "Function.h"
#include "LinMods.h"
#include "Logger.h"
#include "Node.h"
#include "numeric"
#include "Objective.h"
#include "Operations.h"
#include "Option.h"
#include "QuadraticFunction.h"
#include "ProblemSize.h"
#include "Relaxation.h"
#include "SolutionPool.h"
#include "Variable.h"

using namespace Minotaur;

const std::string PowHandler::me_ = "PowHandler: ";

PowHandler::PowHandler(EnvPtr env, ProblemPtr problem)
  : bStats_(), pStats_(), sStats_(), LBd_(-1e6), UBd_(1e6), bTol_(1e-6),
    env_(env), orig_(problem), optCuts_(), aTol_(1e-6), rTol_(1e-6),
    eTol_(1e-6), vTol_(1e-6), p_(problem), log_(env->getLogger()),
    tmpX_(), grad_()
{
  tmpX_.assign(problem->getNumVars(), 0.0);
  grad_.assign(problem->getNumVars(), 0.0);
}

PowHandler::~PowHandler()
{
  for (auto it = consd_.begin(); it != consd_.end(); ++it) {
    delete *it;
  }
}

void continuedFraction(double val, int64_t &num, int64_t &den,
                        double relTol = 1e-9, double absTol = 1e-12,
                        int64_t maxDen = 1000000) {
  bool neg = val < 0;
  val = std::fabs(val);

  // Handle zero
  if (val < absTol) {
    num = 0;
    den = 1;
    return;
  }

  int64_t h1 = 1, h2 = 0, k1 = 0, k2 = 1;
  double x = val;

  int64_t bestNum = 0, bestDen = 1;  // fallback
  bool found = false;

  for (int i = 0; i < 64; ++i) {
    int64_t a = static_cast<int64_t>(std::floor(x));
    int64_t h = a * h1 + h2;
    int64_t k = a * k1 + k2;

    if (k > maxDen) break;

    h2 = h1; h1 = h;
    k2 = k1; k1 = k;

    double approx = static_cast<double>(h1) / static_cast<double>(k1);
    double relErr = std::fabs(val - approx) / val;
    double absErr = std::fabs(val - approx);

    // First convergent inside rel_tol is the simplest possible -- stop immediately
    if (relErr <= relTol) {
      bestNum = h1;
      bestDen = k1;
      found = true;
      break;  // simplest within tolerance
    }

    // Exact match (input was a clean rational like 67/100)
    if (absErr < absTol) {
      bestNum = h1;
      bestDen = k1;
      found = true;
      break;
    }

    double frac = x - static_cast<double>(a);
    if (frac < 1e-13) break;  // next CF term would be infinite
    x = 1.0 / frac;
  }

  if (!found) {
    // best accuracy we could get within maxDen
    bestNum = h1;
    bestDen = k1;
  }

  // Reduce the fraction using GCD
  int64_t g = std::gcd(bestNum, bestDen);
  if (g > 0) {
    bestNum /= g;
    bestDen /= g;
  }

  num = neg ? -bestNum : bestNum;
  den = bestDen;
}

double PowHandler::getViol_(const PowCons &cd, const DoubleVector &x) const {
  double xval = x[cd.riv->getIndex()];
  double fhat = x[cd.rov->getIndex()];
  double fval = getF_(xval, cd.k, cd.type);
  double absViol = std::abs(fhat - fval); 
  double relViol = absViol;
  if (std::abs(fval) + absViol > 1.0) {
    relViol = absViol / (std::abs(fval) + absViol);
  }
  return relViol;
}

double PowHandler::getF_(double xv, double k, PowType type) const {
  if (xv >= 0.0) return std::pow(xv, k);
  if (type == POS_EO || type == NEG_EO) return std::pow(std::abs(xv), k);
  if (type == POS_OO || type == NEG_OO) return -std::pow(std::abs(xv), k);
  return 0.0;
}

double PowHandler::getDf_(double xv, double k, PowType type) const {
  if (xv >= 0.0) return k * std::pow(xv, k - 1.0);

  // Assert domain safety
  assert(type != POS_OE && type != NEG_OE && "PowHandler: Invalid negative 1st derivative!");

  // Manual sign adjustment based on numerator parity
  // Even Numerator (p): f(x) = |x|^k -> f'(x) = -k|x|^(k-1)
  if (type == POS_EO || type == NEG_EO) {
      return -k * std::pow(std::abs(xv), k - 1.0);
  }
  
  // Odd Numerator (p): f(x) = -|x|^k -> f'(x) = +k|x|^(k-1)
  // The derivative of an odd function in the negative domain is positive!
  return k * std::pow(std::abs(xv), k - 1.0);
}


double PowHandler::getD2f_(double xv, double k, PowType type) const {
  // 1. Positive domain is always positive curvature (for k > 1)
  if (xv >= 0.0) return k * (k - 1.0) * std::pow(xv, k - 2.0);

  assert(type != POS_OE && type != NEG_OE && "PowHandler: Invalid negative 2nd derivative!");

  double val = k * (k - 1.0) * std::pow(std::abs(xv), k - 2.0);

  // 2. Even Numerators: f(x) = |x|^k 
  // f''(x) is POSITIVE in the negative domain
  if (type == POS_EO || type == NEG_EO) {
      return val;
  }
  
  // 3. Odd Numerators: f(x) = -|x|^k
  // f''(x) is NEGATIVE in the negative domain
  return -val;
}


double PowHandler::safeX_(double xv, double k) const {
  if ((k < 0.0 || (k > 0.0 && k < 1.0)) && std::abs(xv) < 1e-15) { 
      return (xv >= 0.0) ? 1e-15 : -1e-15; 
  }
  return xv;
}


double PowHandler::computeWStar_(double k) const {
  // Solves G(s) = k*s^{k-1} + (k-1)*s^k - 1 = 0 for s in (0,1).s=x/l or x/u
  // True root is alpha = -s (odd/odd sign convention applied by caller).
  //
  // Corner cases:
  if (k <= 0.0) return 0.0;                    // undefined / not applicable
  if (std::abs(k - 1.0) < 1e-9) return 0.0;     // k=1: tangent-secant degenerate, no crossing

  auto G = [k](double s) {
    return k * std::pow(s, k - 1.0) + (k - 1.0) * std::pow(s, k) - 1.0;
  };

  double lo = 1e-12, hi = 1.0 - 1e-12;
  double glo = G(lo), ghi = G(hi);

  // Defensive check -- should never trigger given the proof above, but
  // guards against pathological floating point behavior at extreme k.
  if (glo * ghi > 0.0) {
    return -1.0; // signal: no sign change found, caller should handle/log
  }

  double mid = 0.5 * (lo + hi);
  for (int i = 0; i < 100; ++i) {
    mid = 0.5 * (lo + hi);
    double gmid = G(mid);

    if (std::abs(gmid) < 1e-14 || (hi - lo) < 1e-14) {
      return -mid;
    }
    if ((gmid > 0.0) == (glo > 0.0)) {
      lo = mid; glo = gmid;
    } else {
      hi = mid; ghi = gmid;
    }
  }
  return -mid;
}

double PowHandler::computeWCross_(double n) const {
  // Solves h(w) = |w|^n - n*w + n - 1 = 0 for w in (-1, 0). w=x/l or x/u
  // w = x*/l, where x* is where the tangent to |x|^n at x=l (l<0)
  // recrosses the curve on the positive branch.
  if (n <= 0.0 || n >= 1.0) return 0.0; // out of scope for this construction

  auto h = [n](double w) {
    return std::pow(-w, n) - n * w + n - 1.0;  // -w > 0 for w in (-1,0)
  };

  double lo = -1.0 + 1e-12, hi = -1e-12;
  double hlo = h(lo), hhi = h(hi);
  // Proven: h(-1)=2n>0, h(0^-)=n-1<0, so hlo>0, hhi<0 -- sign change guaranteed.
  if (hlo * hhi > 0.0) return 0.0; // defensive; shouldn't trigger

  double w = 0.5 * (lo + hi);
  for (int i = 0; i < 100; ++i) {
    w = 0.5 * (lo + hi);
    double hw = h(w);
    if (std::abs(hw) < 1e-14 || (hi - lo) < 1e-14) break;
    if ((hw > 0.0) == (hlo > 0.0)) { lo = w; hlo = hw; }
    else                            { hi = w; hhi = hw; }
  }
  return w; // x*/l ratio, in (-1,0)
}



void PowHandler::relaxSlot_(PowCons &cd, RelaxationPtr rel, size_t slot, ModVector &mods) {
  if (slot >= cd.secCons.size() || cd.secCons[slot] == nullptr) return;
  ConstraintPtr target = cd.secCons[slot];
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(cd.rov, 1.0);
  rel->changeConstraint(target, lf, -INFINITY, INFINITY);
  LinConModPtr lcmod = (LinConModPtr) new LinConMod(target, lf, -INFINITY, INFINITY);
  mods.push_back(lcmod);
}



// ----------------------------------------------------------------------------
// NODE PROCESSING
// ----------------------------------------------------------------------------

void PowHandler::relaxInitInc(RelaxationPtr rel, SolutionPool *, bool *is_inf) {
  ModVector dummy_mods; 
  for (auto it = consd_.begin(); it != consd_.end(); ++it) {
    (*it)->riv = rel->getVariable((*it)->iv->getIndex());
    (*it)->rov = rel->getVariable((*it)->ov->getIndex());
    linearize_(**it, rel, dummy_mods, true);
  }
  *is_inf = false;
}

void PowHandler::relaxNodeInc(NodePtr node, RelaxationPtr rel, bool *is_infeasible) {
  ModVector mods;
  for (auto it = consd_.begin(); it != consd_.end(); ++it) {
    (*it)->riv = rel->getVariable((*it)->iv->getIndex());
    (*it)->rov = rel->getVariable((*it)->ov->getIndex());
    linearize_(**it, rel, mods, false);
  }
  *is_infeasible = false;
}





void PowHandler::linearize_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init) {
  double k = cd.k;
  
#if SPEW
  log_->msgStream(LogDebug1) << me_ << "Linearizing constraint: type=" << cd.type 
                             << std::endl;
#endif

  switch (cd.type) {
    case POS_EO:
      if (k >= 1.0) {
#if SPEW
        log_->msgStream(LogDebug1) << me_ << "Routing to buildPosEO_GT1_(x^4 type)" << std::endl;
#endif
        buildPosEO_GT1_(cd, rel, mods, init);
      } else {
#if SPEW
        log_->msgStream(LogDebug1) << me_ << "Routing to buildPosEO_LT1_( x^(2/3) type)" << std::endl;
#endif
        buildPosEO_LT1_(cd, rel, mods, init);
      }
      break;

    case POS_OO: 
      if (k >= 1.0) {
#if SPEW
        log_->msgStream(LogDebug1) << me_ << "Routing to buildPosOO_GT1_(x^3 type )" << std::endl;
#endif
        buildPosOO_GT1_(cd, rel, mods, init);
      } else {
#if SPEW
        log_->msgStream(LogDebug1) << me_ << "Routing to buildPosOO_LT1_(x^(1/3) type )" << std::endl;
#endif
        buildPosOO_LT1_(cd, rel, mods, init);
      }
      break;

    case POS_OE: 
      if (k >= 1.0) {
#if SPEW
        log_->msgStream(LogDebug1) << me_ << "Routing to buildPosOE_GT1_(x^(3/2) type)" << std::endl;
#endif
        buildPosOE_GT1_(cd, rel, mods, init);
      } else {
#if SPEW
        log_->msgStream(LogDebug1) << me_ << "Routing to buildPosOE_LT1_(root x type)" << std::endl;
#endif
        buildPosOE_LT1_(cd, rel, mods, init);
      }
      break;

    case NEG_EO: 
#if SPEW
      log_->msgStream(LogDebug1) << me_ << "Routing to buildNegEO_ (1/x^2 type)" << std::endl;
#endif
      buildNegEO_(cd, rel, mods, init); 
      break;

    case NEG_OO: 
#if SPEW
      log_->msgStream(LogDebug1) << me_ << "Routing to buildNegOO_(1/x type)" << std::endl;
#endif
      buildNegOO_(cd, rel, mods, init); 
      break;

    case NEG_OE: 
#if SPEW
      log_->msgStream(LogDebug1) << me_ << "Routing to buildNegOE_ 1/(root x) type" << std::endl;
#endif
      buildNegOE_(cd, rel, mods, init); 
      break;
  }
}



void PowHandler::buildPosEO_GT1_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init) {
  double xlb = cd.riv->getLb(), xub = cd.riv->getUb();
  if (xub - xlb < 1e-6) return;
  addSecXSq_(cd, rel, xlb, xub, 1, mods, init);   // secant: always update, bound-dependent
  if (init) {                                      // tangent adding only at root
    double pts[3] = {xlb, xub, xlb + (xub - xlb) / 2.0};
    for (int i = 0; i < 3; ++i) addTanXSq_(cd, rel, pts[i], -1);
  }
}

void PowHandler::buildPosOE_GT1_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init) {
  double xlb = cd.riv->getLb(), xub = cd.riv->getUb();
  if (xub - xlb < 1e-6) return;
  addSecXSq_(cd, rel, xlb, xub, 1, mods, init);   // secant: always update, bound-dependent
  if (init) {                                      // tangent adding only at root
    double pts[3] = {xlb, xub, xlb + (xub - xlb) / 2.0};
    for (int i = 0; i < 3; ++i) addTanXSq_(cd, rel, pts[i], -1);
  }
}


void PowHandler::buildPosOO_GT1_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init) {
  double xlb = cd.riv->getLb();
  double xub = cd.riv->getUb();

  if (xub - xlb < 1e-6) return;

  if (xlb >= -1e-6) {
    // Standard convex case
    addSecXSq_(cd, rel, xlb, xub, 1, mods, init);
    if (init) {
      addTanXSq_(cd, rel, xlb, -1);
      addTanXSq_(cd, rel, xub, -1);
      addTanXSq_(cd, rel, xlb + (xub - xlb) / 2.0, -1);
    }

  } else if (xub <= 1e-6) {
    // Standard concave case
    addSecXSq_(cd, rel, xlb, xub, -1, mods, init);
    if (init) {
      addTanXSq_(cd, rel, xlb, 1);
      addTanXSq_(cd, rel, xub, 1);
      addTanXSq_(cd, rel, xlb + (xub - xlb) / 2.0, 1);
    }

  } else {
    // INFLECTION ENVELOPE (unchanged)
    double w = computeWStar_(cd.k);
    double t_over = w * xub;
    addTanXSq_(cd, rel, t_over, +1);
    double t_under = w * xlb;
    addTanXSq_(cd, rel, t_under, -1);
#if SPEW
    log_->msgStream(LogDebug1) << me_ << "Added Inflection Envelope tangents for x^" << cd.k
                               << " | w=" << w << std::endl;
#endif
  }
}

void PowHandler::buildPosOE_LT1_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init) {
  double xlb = cd.riv->getLb(), xub = cd.riv->getUb();
  if (xub - xlb < 1e-6) return;
  addSecXSq_(cd, rel, xlb, xub, -1, mods, init);

  if(init)
  {
    double pts[3] = {xlb, xub, xlb + (xub - xlb) / 2.0};
    for (int i = 0; i < 3; ++i) addTanXSq_(cd, rel, pts[i], +1);
  }

}




void PowHandler::buildPosOO_LT1_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init) {
  double xlb = cd.riv->getLb();
  double xub = cd.riv->getUb();
  if (xub - xlb < 1e-6) return;

  if (xlb >= -1e-6) {
    // Strictly Concave Domain (Right of Y-axis)
    addSecXSq_(cd, rel, xlb, xub, -1, mods, init);

    if(init)
    {
    addTanXSq_(cd, rel, xlb, 1);
    addTanXSq_(cd, rel, xub, 1);
    addTanXSq_(cd, rel, xlb + (xub - xlb) / 2.0, 1);

    }
   
  } else if (xub <= 1e-6) {
    // Strictly Convex Domain (Left of Y-axis)
    addSecXSq_(cd, rel, xlb, xub, 1, mods, init);
    if(init)
    {
    addTanXSq_(cd, rel, xlb, -1);
    addTanXSq_(cd, rel, xub, -1);
    addTanXSq_(cd, rel, xlb + (xub - xlb) / 2.0, -1);

    }
    
  } else {
    // --- CROSSING ZERO: INFLECTION ENVELOPE (k < 1) ---
    // Curvature is inverted compared to k > 1 (Convex on left, Concave on right).
    // Compute the ratio w
    double w = computeWStar_(cd.k);
    
    // 1. Lower Bounding Line (Underestimator)
    // Tangent point t_under is on the negative side (< 0), anchored at positive bound xub > 0
    double t_under = w * xub; 
    addTanXSq_(cd, rel, t_under, -1);

    // 2. Upper Bounding Line (Overestimator)
    // Tangent point t_over is on the positive side (> 0), anchored at negative bound xlb < 0
    double t_over = w * xlb;  
    addTanXSq_(cd, rel, t_over, 1);

#if SPEW
    log_->msgStream(LogDebug1) << me_ << "Added Inflection Envelope tangents for k < 1 (x^" << cd.k 
                               << ") | w_star=" << w 
                               << " | t_under=" << t_under << ", t_over=" << t_over << std::endl;
#endif
  }
}



void PowHandler::buildPosEO_LT1_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init) {
  double xlb = cd.riv->getLb();
  double xub = cd.riv->getUb();
  if (xub - xlb < 1e-6) return;
  const double eps = 1e-6;

  const size_t SLOT_LEFT = 0, SLOT_RIGHT = 1, SLOT_UPPER = 2;

  if (xlb >= -eps) {
    addSecXSq_(cd, rel, xlb, xub, -1, mods, init, SLOT_LEFT);
    if (!init) { relaxSlot_(cd, rel, SLOT_RIGHT, mods); relaxSlot_(cd, rel, SLOT_UPPER, mods); }
    if (init) {
      if (xlb > eps) addTanXSq_(cd, rel, xlb, 1);
      addTanXSq_(cd, rel, xub, 1);
      addTanXSq_(cd, rel, xlb + (xub - xlb) / 2.0, 1);
    }

  } else if (xub <= eps) {
    addSecXSq_(cd, rel, xlb, xub, -1, mods, init, SLOT_RIGHT);
    if (!init) { relaxSlot_(cd, rel, SLOT_LEFT, mods); relaxSlot_(cd, rel, SLOT_UPPER, mods); }
    if (init) {
      addTanXSq_(cd, rel, xlb, 1);
      if (xub < -eps) addTanXSq_(cd, rel, xub, 1);
      addTanXSq_(cd, rel, xlb + (xub - xlb) / 2.0, 1);
    }

  } else {
    // CROSSING ZERO
    addSecXSq_(cd, rel, xlb, 0.0, -1, mods, init, SLOT_LEFT);
    addSecXSq_(cd, rel, 0.0, xub, -1, mods, init, SLOT_RIGHT);

    double w_cross = computeWCross_(cd.k);
    double L = std::abs(xlb);
    double t = 0.0;
    bool tangentChosen;

    if (L > xub) {
      t = w_cross * xlb;
      tangentChosen = (t >= xub);
    } else {
      t = xub * w_cross;
      tangentChosen = (std::abs(t) >= L);
    }

    if (tangentChosen) {
      // Tangent is a permanently-valid global cut once added -- add it
      // whichever node first reaches this regime; relax the upper-secant
      // slot so a stale extrapolated secant can't linger.
      addTanXSq_(cd, rel, (L > xub) ? xlb : xub, 1);
      if (!init) relaxSlot_(cd, rel, SLOT_UPPER, mods);
    } else {
      addSecXSq_(cd, rel, xlb, xub, 1, mods, init, SLOT_UPPER);
    }

#if SPEW
    log_->msgStream(LogDebug1) << me_ << "Added V type cuts for x^" << cd.k
                               << " | w_cross=" << w_cross << " | t=" << t << std::endl;
#endif
  }
}

void PowHandler::buildNegOE_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init) {
  // n = -p/q, p odd, q even => domain x >= 0 only.
  // f(x) = x^n: convex, strictly decreasing, pole (f->+inf) as x->0+.
  double xlb = cd.riv->getLb(), xub = cd.riv->getUb();
  if (xub - xlb < 1e-6) return;
  const double eps = 1e-6;

  if(init){
   double xlb_safe = std::max(xlb, eps); // can't differentiate AT the pole
  double pts[3] = {xlb_safe, xub, xlb_safe + (xub - xlb_safe) / 2.0};
  for (int i = 0; i < 3; ++i) addTanXSq_(cd, rel, pts[i], -1);
  
  }
   // The secant (upper bound)  // strictly between bounds
  if (xlb > eps) {
    addSecXSq_(cd, rel, xlb, xub, 1, mods, init);

  }
#if SPEW
  else {
    log_->msgStream(LogDebug1) << me_ << "NegOE:(x^(-1/2)) xlb within eps of pole, "
        "skipping secant (y left unbounded above near pole)" << std::endl;
  }
#endif
}

void PowHandler::buildNegEO_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init) {
  // n = -p/q, p even, q odd => domain x != 0, f(x)=|x|^n > 0 everywhere,
  // pole from both sides, each side individually convex.
  double xlb = cd.riv->getLb(), xub = cd.riv->getUb();
  if (xub - xlb < 1e-6) return;
  const double eps = 1e-6;

  if (xlb >= eps || xub <= -eps) {
    // --- Single branch, entirely right or entirely left of the pole ---
    bool rightSide = (xlb >= eps);
    double near = rightSide ? xlb : xub;   // endpoint closest to the pole
    double far  = rightSide ? xub : xlb;
    double nearSafe = rightSide ? std::max(near, eps) : std::min(near, -eps);
    if(init)
    {
     double pts[3] = {nearSafe, far, nearSafe + (far - nearSafe) / 2.0};
      for (int i = 0; i < 3; ++i) addTanXSq_(cd, rel, pts[i], -1);

    }
       if (std::abs(near) > eps) {
      addSecXSq_(cd, rel, xlb, xub, 1, mods, init);
    }
#if SPEW
    else {
      log_->msgStream(LogDebug1) << me_ << "NegEO: near-pole endpoint "
          "inside eps, skipping secant " << std::endl;
    }
#endif

  } else {

    if(init)
    {
      addTanXSq_(cd,rel,0,-1);

    }
      
#if SPEW
    log_->msgStream(LogDebug1) << me_ << "NegEO: domain has 0 "
        "no valid envelope exists; added weak y>=0 bound only. Recommend "
        "branching to split the domain at x=0." << std::endl;
#endif
  }
}

void PowHandler::buildNegOO_(PowCons &cd, RelaxationPtr rel, ModVector &mods, bool init) {
  // n = -p/q, p,q odd => f odd, domain x != 0. Convex+positive on x>0,
  // concave+negative on x<0.
  double xlb = cd.riv->getLb(), xub = cd.riv->getUb();
  if (xub - xlb < 1e-6) return;
  const double eps = 1e-6;

  if (xlb >= eps) {
    // Entirely right of pole: convex, positive -- same as NegOE.
    if (init) {
      double xlb_safe = std::max(xlb, eps);
      double pts[3] = {xlb_safe, xub, xlb_safe + (xub - xlb_safe) / 2.0};
      for (int i = 0; i < 3; ++i) addTanXSq_(cd, rel, pts[i], -1);
    }
    addSecXSq_(cd, rel, xlb, xub, 1, mods, init);

  } else if (xub <= -eps) {
    // Entirely left of pole: concave, negative. Tangent/secant roles
    // flip relative to the right branch.
    if (init) {
      double xub_safe = std::min(xub, -eps);
      double pts[3] = {xlb, xub_safe, xlb + (xub_safe - xlb) / 2.0};
      for (int i = 0; i < 3; ++i) addTanXSq_(cd, rel, pts[i], 1);
    }
    addSecXSq_(cd, rel, xlb, xub, -1, mods, init);

  } else {
    
#if SPEW
    log_->msgStream(LogDebug1) << me_ << "NegOO: domain has pole "
        "(y unbounded both directions)  no valid linear envelope "
        "exists. Recommend branching to split "
         << std::endl;
#endif
  }
}


void PowHandler::addSecXSq_(PowCons &cd, RelaxationPtr rel, double xlb, double xub,
                             int bound_dir, ModVector &mods, bool init,
                             size_t slot) {
  double fxlb = getF_(xlb, cd.k, cd.type);
  double fxub = getF_(xub, cd.k, cd.type);
  double m = 0.0;

  if (xlb <= -1e15 || xub >= 1e15) {
    return;
  }

  if (xub - xlb > 1e-6) m = (fxub - fxlb) / (xub - xlb);
  double intercept = fxlb - m * xlb;
  if (std::abs(m) < 1e-9) m = 0.0;

  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(cd.rov, 1.0);
  lf->addTerm(cd.riv, -m);

  double lb = (bound_dir == 1) ? -INFINITY : intercept;
  double ub = (bound_dir == -1) ? INFINITY : intercept;

  bool slotExists = (slot < cd.secCons.size() && cd.secCons[slot] != nullptr);

  if (!slotExists) {
    // First time this slot is needed -- create it now, whether or not
    // this is the init call. A regime that never touched addSecXSq_
    // before (e.g. the crossing-zero branch, which only adds tangents)
    // can still reach here later once bounds shrink into a single-sided
    // regime that does need a secant.
    FunctionPtr f = (FunctionPtr) new Function(lf);
    ConstraintPtr newCon = rel->newConstraint(f, lb, ub);
    if (cd.secCons.size() <= slot) {
      cd.secCons.resize(slot + 1, nullptr);
    }
    cd.secCons[slot] = newCon;
#if SPEW
    log_->msgStream(LogDebug1) << me_ << "Added new secant (slot " << slot
                               << ") at node for x^" << cd.k << std::endl;
    newCon->write(log_->msgStream(LogDebug2));
#endif
  } else {
    ConstraintPtr target = cd.secCons[slot];
    rel->changeConstraint(target, lf, lb, ub);
    LinConModPtr lcmod = (LinConModPtr) new LinConMod(target, lf, lb, ub);
    mods.push_back(lcmod);
#if SPEW
    log_->msgStream(LogDebug1) << me_ << "Modified secant (slot " << slot
                               << ") at node for x^" << cd.k << std::endl;
    target->write(log_->msgStream(LogDebug2));
#endif
  }
}

void PowHandler::addTanXSq_(PowCons &cd, RelaxationPtr rel, double xv, int bound_dir) {
  double fx = getF_(xv, cd.k, cd.type);
  double dfx = getDf_(xv, cd.k, cd.type);
  if (!std::isfinite(fx) || !std::isfinite(dfx)) {
  log_->msgStream(LogError)
      << me_ << "Invalid tangent at x=" << xv
      << " k=" << cd.k
      << " fx=" << fx
      << " dfx=" << dfx << std::endl;
  return;
}
  double rhs = fx - dfx * xv;

if (!std::isfinite(rhs)) {
  log_->msgStream(LogError)
      << me_ << "Invalid tangent RHS at x=" << xv
      << " rhs=" << rhs << std::endl;
  return;
}
  LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
  lf->addTerm(cd.rov, 1.0);
  lf->addTerm(cd.riv, -dfx);

  double lb = (bound_dir == 1) ? -INFINITY : rhs;
  double ub = (bound_dir == -1) ? INFINITY : rhs;
  FunctionPtr f = (FunctionPtr) new Function(lf);
  ConstraintPtr cons = rel->newConstraint(f, lb, ub);
  cd.linCons.push_back(cons);

  #if SPEW
  log_->msgStream(LogDebug1) << me_ << "Added tangent cut at x=" << xv 
                             << " | Bounds: [" << lb << ", " << ub << "]" << std::endl;
   cons->write(log_->msgStream(LogDebug1)); 

#endif
}




// ADD CONSTRAINT & SEPARATION


void PowHandler::addConstraint(ConstraintPtr newcon, ConstVariablePtr ivar, ConstVariablePtr ovar, double degree, char sense) {
  Handler::addConstraint(newcon); 

  int64_t p_64, q_64;
  continuedFraction(degree, p_64, q_64);
  int p = static_cast<int>(p_64), q = static_cast<int>(q_64);

  PowType t;
  bool is_pos = (degree > 0.0);
  bool p_odd = (std::abs(p) % 2 != 0);
  bool q_odd = (std::abs(q) % 2 != 0);

  if (is_pos) {
    if (p_odd && q_odd)        t = POS_OO;
    else if (!p_odd && q_odd)  t = POS_EO;
    else if (p_odd && !q_odd)  t = POS_OE;
  } else {
    if (p_odd && q_odd)        t = NEG_OO;
    else if (!p_odd && q_odd)  t = NEG_EO;
    else if (p_odd && !q_odd)  t = NEG_OE;
    else { assert(false);log_->msgStream(LogError)<<me_<<"invalid type"<<std::endl;  }
  }

  VariablePtr vivar = (VariablePtr) ivar;
  VariablePtr vovar = (VariablePtr) ovar;
  consd_.push_back(new PowCons(newcon, vivar, vovar, degree, p, q, t, sense));
}

PowHandler::PowCurvature PowHandler::getCurvature_(const PowCons &cd, double xlb, double xub) const {
  const double eps = 1e-6;
  switch (cd.type) {
    case POS_EO:
      if (cd.k >= 1.0) return PC_CONVEX;
      if (xlb >= -eps || xub <= eps) return PC_CONCAVE;
      return PC_MIXED;

    case POS_OO:
      if (xlb >= -eps) return (cd.k >= 1.0) ? PC_CONVEX : PC_CONCAVE;
      if (xub <= eps)  return (cd.k >= 1.0) ? PC_CONCAVE : PC_CONVEX;
      return PC_MIXED;

    case POS_OE:
      return (cd.k >= 1.0) ? PC_CONVEX : PC_CONCAVE;

    case NEG_OE:
      return PC_CONVEX;

    case NEG_EO:
      if (xlb >= eps || xub <= -eps) return PC_CONVEX;
      return PC_MIXED;

    case NEG_OO:
      if (xlb >= eps) return PC_CONVEX;
      if (xub <= -eps) return PC_CONCAVE;
      return PC_MIXED;
  }
  return PC_MIXED;
}

void PowHandler::addCut_(VariablePtr x, VariablePtr y, double xval, double yval,
                          RelaxationPtr rel, bool &ifcuts, PowCons *cd_) {
  ifcuts = false;

  double xlb = cd_->riv->getLb(), xub = cd_->riv->getUb();
  PowCurvature curv = getCurvature_(*cd_, xlb, xub);
  if (curv == PC_MIXED) return;

  double xv = safeX_(xval, cd_->k);
  double fval  = getF_(xv, cd_->k, cd_->type);
  double deriv = getDf_(xv, cd_->k, cd_->type);
  if (!std::isfinite(fval) || !std::isfinite(deriv)) return;

  bool convex = (curv == PC_CONVEX);
  double violation = convex ? (fval - yval) : (yval - fval);

  if (violation > aTol_) {
    double rhs = fval - deriv * xv;
    LinearFunctionPtr lf = (LinearFunctionPtr) new LinearFunction();
    lf->addTerm(y, 1.0);
    lf->addTerm(x, -deriv);
    FunctionPtr f = (FunctionPtr) new Function(lf);

    if (convex) {
      rel->newConstraint(f, rhs, INFINITY);
    } else {
      rel->newConstraint(f, -INFINITY, rhs);
    }
    ifcuts = true;
  }
}

void PowHandler::separate(ConstSolutionPtr sol, NodePtr, RelaxationPtr rel, CutManager*,
                           SolutionPoolPtr, ModVector&, ModVector&, bool* sol_found,
                           SeparationStatus* /*status*/) {
  *sol_found = false;
  const double* x = sol->getPrimal();

  for (auto cd : consd_) {
    VariablePtr x_var = rel->getRelaxationVar(cd->iv);
    VariablePtr y_var = rel->getRelaxationVar(cd->ov);
    if (!x_var || !y_var) continue;

    double xval = x[x_var->getIndex()];
    double yval = x[y_var->getIndex()];
    bool added = false;
    addCut_(x_var, y_var, xval, yval, rel, added, cd);
    if (added) {
      *sol_found = true;
      ++sStats_.tangentcuts;
    }
  }
}

// FEASIBILITY, BOUNDS, & PRESOLVE

bool PowHandler::isFeasible(ConstSolutionPtr sol, RelaxationPtr rel, bool& isfeas, double& inf_meas) {
  return isFeasible_(sol, rel, isfeas, inf_meas);
}

bool PowHandler::isFeasible_(ConstSolutionPtr sol, RelaxationPtr, bool &isfeas, double &inf_meas) {
  isfeas = true;
  inf_meas = 0.0;
  const double *primal = sol->getPrimal();
  DoubleVector x(primal, primal + p_->getNumVars());

  for (auto cd : consd_) {
    double viol = getViol_(*cd, x);
    if (viol > eTol_) {
      if (cd->riv->getUb() - cd->riv->getLb() > vTol_) {
        isfeas = false;
        inf_meas += viol;
      }
    }  
  }
  return isfeas;
}

void PowHandler::getBranchingCandidates(RelaxationPtr, const DoubleVector &x, ModVector &, BrVarCandSet &cands, BrCandVector &, bool &is_inf) {
  std::map<ConstVariablePtr, double> allCands;
  is_inf = false;

  for (auto cd : consd_) {
    double curviol = getViol_(*cd, x);
    if (curviol > eTol_) {
      ConstVariablePtr v = cd->riv;
      if (allCands.find(v) == allCands.end()) allCands[v] = curviol;
      else allCands[v] += curviol;
    }
  }

  for (auto it = allCands.begin(); it != allCands.end(); ++it) {
    BrVarCandPtr br_can = (BrVarCandPtr) new BrVarCand(it->first, it->first->getIndex(), it->second, it->second);
    cands.insert(br_can);
  }
}

ModificationPtr PowHandler::getBrMod(BrCandPtr cand, DoubleVector &x, RelaxationPtr, BranchDirection brdir) {
  LinModsPtr lmods = (LinModsPtr) new LinMods();
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();
  double xval = x[v->getIndex()];
  double value = xval;
  double len = v->getUb() - v->getLb();

  if (v->getLb() < -1e-6 && v->getUb() > 1e-6) value = 0.0; 
  else {
    double minFromBds = 0.1;
    if (value < v->getLb() + minFromBds * len) value = v->getLb() + minFromBds * len;
    else if (value > v->getUb() - minFromBds * len) value = v->getUb() - minFromBds * len;
  }

  VarBoundModPtr bmod = (brdir == DownBranch) ? (VarBoundModPtr) new VarBoundMod(v, Upper, value) : (VarBoundModPtr) new VarBoundMod(v, Lower, value);
  lmods->insert(bmod);
  return lmods;
}

Branches PowHandler::getBranches(BrCandPtr cand, DoubleVector &x, RelaxationPtr rel, SolutionPoolPtr) {
  BrVarCandPtr vcand = dynamic_cast<BrVarCand *>(cand);
  VariablePtr v = vcand->getVar();
  VariablePtr v2 = rel->getOriginalVar(v); 
  
  PowConsPtr cd = nullptr;
  for (auto c : consd_) {
    if (c->iv == v2) { cd = c; break; }
  }

  double xval = x[v->getIndex()];
  double value = xval;
  double lb = v->getLb(), ub = v->getUb(), len = ub - lb;

  if (cd) {
    if (cd->type == POS_EO || cd->type == POS_OE || cd->type == NEG_EO) {
      if (value < 0.0) value = 0.0;
    }
    if (lb < -1e-6 && ub > 1e-6) {
      if (cd->type == POS_EO || cd->type == NEG_EO || cd->type == NEG_OE) value = 0.0;
    }
  }

  double minFromBds = 0.1;
  if (value < lb + minFromBds * len) value = lb + minFromBds * len;
  else if (value > ub - minFromBds * len) value = ub - minFromBds * len;

  Branches branches = (Branches) new BranchPtrVector();
  BranchPtr branchDown = new Branch();
  branchDown->addPMod(new VarBoundMod(v2, Upper, value));
  branchDown->addRMod(new VarBoundMod(v, Upper, value));
  branchDown->setActivity(value);
  branches->push_back(branchDown);

  BranchPtr branchUp = new Branch();
  branchUp->addPMod(new VarBoundMod(v2, Lower, value));
  branchUp->addRMod(new VarBoundMod(v, Lower, value));
  branchUp->setActivity(value);
  branches->push_back(branchUp);

  vcand->setNumBranches(2);
  return branches;
}

BranchPtr PowHandler::doBranch_(BranchDirection UpOrDown, ConstVariablePtr v, double bvalue) {
  BranchPtr branch = new Branch();
  std::fill(tmpX_.begin(), tmpX_.end(), 0.0);
  std::fill(grad_.begin(), grad_.end(), 0.0);

  VariablePtr v_mut = const_cast<Variable *>(v);
  BoundType type = (UpOrDown == DownBranch) ? Upper : Lower;
  VarBoundModPtr pmod = new VarBoundMod(v_mut, type, bvalue);
  
  LinModsPtr linmodsP = new LinMods();
  linmodsP->insert(pmod);
  branch->addPMod(linmodsP);
  return branch;
}

void PowHandler::dupRows_(bool *changed) {
  UInt n = p_->getNumVars(), m = consd_.size(); 
  int err = 0;
  DoubleVector r1, r2, h1, h2;
  
  for (UInt i = 0; i < n; ++i) {
    r1.push_back(1.0 + ((double)rand() / RAND_MAX) * 10.0);
    r2.push_back(1.0 + ((double)rand() / RAND_MAX) * 10.0);
  }

  for (UInt i = 0; i < m; ++i) {
    h1.push_back(consd_[i]->con->getActivity(&(r1[0]), &err));
    h2.push_back(consd_[i]->con->getActivity(&(r2[0]), &err));
  }

  for (UInt i = 0; i < m; ++i) {
    if (h1[i] >= 1e29) continue;
    for (UInt j = i + 1; j < m; ++j) {
      if (h1[j] >= 1e29) continue;
      if (fabs(h1[i] - h1[j]) < 1e-10 && fabs(h2[i] - h2[j]) < 1e-10) {
        if (treatDupRows_(consd_[i]->con, consd_[j]->con, 1.0, changed)) h1[j] = h2[j] = 1e30;
      }
    }
  }
}

bool PowHandler::treatDupRows_(ConstraintPtr c1, ConstraintPtr c2, double, bool *changed) {
  PowConsPtr d1 = nullptr, d2 = nullptr;
  for (auto &cd : consd_) {
    if (cd->con == c1) d1 = cd;
    if (cd->con == c2) d2 = cd;
  }

  if (!d1 || !d2 || d1->iv != d2->iv || d1->ov != d2->ov || d1->sense != d2->sense) return false;

  double lb = std::max(c1->getLb(), c2->getLb());
  double ub = std::min(c1->getUb(), c2->getUb());

  p_->changeBound(c1, lb, ub);
  c2->setBFlag(true);
  p_->markDelete(c2);
  ++pStats_.conDel;

  *changed = true;
  return true;
}

bool PowHandler::propPowBnds_(PowConsPtr cd, bool *changed) {
  if (cd->type == POS_OE || cd->type == NEG_OE) {
    if (cd->iv->getUb() < 0.0) return true; 
    if (cd->iv->getLb() < 0.0) {
       if (updatePBnds_(cd->iv, 0.0, cd->iv->getUb(), changed) < 0) return true;
    }
  }
 // if (cd->type == POS_OO || cd->type == POS_EO) {
 //    if (cd->iv->getUb() < 0.0) return true;
 //    if (cd->iv->getLb() < 0.0) {
 //      if (updatePBnds_(cd->iv, 0.0, cd->iv->getUb(), changed) < 0) return true;
 //    }
 //  }
  switch (cd->type) {
    case POS_OO: 
    case POS_OE: return propPosMonotonicBnds_(cd, changed);
    case POS_EO: return propPosEvenBnds_(cd, changed);    
    case NEG_OO: 
    case NEG_OE: return propNegMonotonicBnds_(cd, changed);
    case NEG_EO: return propNegEvenBnds_(cd, changed);
  }
  return false;
}

bool PowHandler::propPosEvenBnds_(PowConsPtr cd, bool *changed) {
  VariablePtr x = cd->iv, z = cd->ov;
  double k = cd->k, inv_k = 1.0 / k;
  double xlb = x->getLb(), xub = x->getUb();
  
  double min_x = (xlb <= 0.0 && xub >= 0.0) ? 0.0 : std::min(std::abs(xlb), std::abs(xub));
  double max_x = std::max(std::abs(xlb), std::abs(xub));
  if (updatePBnds_(z, std::pow(min_x, k), std::pow(max_x, k), changed) < 0) return true;

  double zub = z->getUb();
  if (zub < 0.0) return true; 
  double new_xub = std::pow(std::max(0.0, zub), inv_k);
  if (updatePBnds_(x, -new_xub, new_xub, changed) < 0) return true;
  
  return false;
}

bool PowHandler::propPosMonotonicBnds_(PowConsPtr cd, bool *changed) {
  VariablePtr x = cd->iv, z = cd->ov;
  double k = cd->k, inv_k = 1.0 / k;
  double xlb = x->getLb(), xub = x->getUb();
  
  if (cd->type == POS_OE) {
      xlb = std::max(0.0, xlb);
      if (xlb > xub) return true; 
  }
  
  double new_zlb = (xlb > -1e15) ? ((xlb < 0) ? -std::pow(-xlb, k) : std::pow(xlb, k)) : -INFINITY;
  double new_zub = (xub < 1e15) ? ((xub < 0) ? -std::pow(-xub, k) : std::pow(xub, k)) : INFINITY;
  if (updatePBnds_(z, new_zlb, new_zub, changed) < 0) return true;

  double zlb = z->getLb(), zub = z->getUb();
  if (cd->type == POS_OE) {
      zlb = std::max(0.0, zlb);
      if (zlb > zub) return true;
  }
  double new_xlb = (zlb > -1e15) ? ((zlb < 0) ? -std::pow(-zlb, inv_k) : std::pow(zlb, inv_k)) : -INFINITY;
  double new_xub = (zub < 1e15) ? ((zub < 0) ? -std::pow(-zub, inv_k) : std::pow(zub, inv_k)) : INFINITY;
  if (updatePBnds_(x, new_xlb, new_xub, changed) < 0) return true;
  
  return false;
}

bool PowHandler::propNegEvenBnds_(PowConsPtr cd, bool *changed) {
  VariablePtr x = cd->iv, z = cd->ov;
  double k = cd->k, inv_k = 1.0 / k;
  double xlb = x->getLb(), xub = x->getUb();
  
  double new_zlb = -INFINITY, new_zub = INFINITY;
  if (xlb < 0.0 && xub > 0.0) { 
    new_zlb = std::pow(std::max(std::abs(xlb), std::abs(xub)), k);
  } else if (xlb >= 0.0) { 
    new_zlb = (xub < 1e15) ? std::pow(xub, k) : 0.0;
    new_zub = std::pow(std::max(xlb, 1e-9), k);
  } else { 
    new_zlb = std::pow(std::abs(std::min(xub, -1e-9)), k);
    new_zub = (xlb > -1e15) ? std::pow(std::abs(xlb), k) : 0.0;
  }
  if (updatePBnds_(z, new_zlb, new_zub, changed) < 0) return true;

  double zlb = z->getLb(), zub = z->getUb();
  if (zub < 0.0) return true; 
  if (zlb > 0.0) {
    double new_xub = std::pow(zlb, inv_k);
    if (updatePBnds_(x, -new_xub, new_xub, changed) < 0) return true;
  }
  return false;
}

bool PowHandler::propNegMonotonicBnds_(PowConsPtr cd, bool *changed) {
  VariablePtr x = cd->iv, z = cd->ov;
  double k = cd->k, inv_k = 1.0 / k;
  double xlb = x->getLb(), xub = x->getUb();
  
  if (cd->type == NEG_OE) {
      xlb = std::max(0.0, xlb);
      if (xlb > xub) return true;
  }
  
  double new_zlb = -INFINITY, new_zub = INFINITY;
  if (xlb >= 0.0) { 
    new_zlb = (xub < 1e15) ? std::pow(xub, k) : 0.0;
    new_zub = std::pow(std::max(xlb, 1e-9), k);
  } else if (xub <= 0.0) { 
    new_zlb = (xlb > -1e15) ? -std::pow(-xlb, k) : 0.0;
    new_zub = -std::pow(-std::min(xub, -1e-9), k);
  }
  if (updatePBnds_(z, new_zlb, new_zub, changed) < 0) return true;

  double zlb = z->getLb(), zub = z->getUb();
  if (cd->type == NEG_OE) {
      zlb = std::max(0.0, zlb);
      if (zlb > zub) return true;
  }
  
  double new_xlb = -INFINITY, new_xub = INFINITY;
  if (zlb > 0.0) {        
    new_xub = std::pow(zlb, inv_k);
    new_xlb = (zub < 1e15) ? std::pow(zub, inv_k) : 0.0;
  } else if (zub < 0.0) { 
    new_xlb = -std::pow(-zub, inv_k);
    new_xub = (zlb > -1e15) ? -std::pow(-zlb, inv_k) : 0.0;
  }
  if (updatePBnds_(x, new_xlb, new_xub, changed) < 0) return true;
  
  return false;
}

int PowHandler::updatePBnds_(VariablePtr p, double newlb, double newub, bool *changed) {
  double lb = p->getLb(), ub = p->getUb();
  
  if (std::abs(newlb - std::round(newlb)) < 1e-4) newlb = std::round(newlb);
  if (std::abs(newub - std::round(newub)) < 1e-4) newub = std::round(newub);
  
  if (p->getType() == Binary || p->getType() == ImplBin || p->getType() == Integer || p->getType() == ImplInt) {
    if (newlb - floor(newlb) < eTol_) newlb = floor(newlb);
    else newlb = ceil(newlb);
    if (ceil(newub) - newub < eTol_) newub = ceil(newub);
    else newub = floor(newub);
  }

  if (newlb > lb) { lb = newlb; *changed = true; ++pStats_.vBnd; }
  if (newub < ub) { ub = newub; *changed = true; ++pStats_.vBnd; }

  if (lb > ub + eTol_) return -1;
  if (fabs(ub - lb) <= vTol_) { double mid = 0.5 * (lb + ub); lb = mid; ub = mid; }
  
  if (*changed) { p->setLb_(lb); p->setUb_(ub); }
  return 0;
}

int PowHandler::updatePBnds_(VariablePtr v, double newlb, double newub, RelaxationPtr rel, bool mod_rel, bool *changed, ModVector &p_mods, ModVector &r_mods) {
  double lb = v->getLb(), ub = v->getUb();
  bool tighten_lb = false, tighten_ub = false;

  if (std::abs(newlb - std::round(newlb)) < 1e-4) newlb = std::round(newlb);
  if (std::abs(newub - std::round(newub)) < 1e-4) newub = std::round(newub);

  if (v->getType() == Binary || v->getType() == ImplBin || v->getType() == Integer || v->getType() == ImplInt) {
    if (newlb - floor(newlb) < eTol_) newlb = floor(newlb);
    else newlb = ceil(newlb);
    if (ceil(newub) - newub < eTol_) newub = ceil(newub);
    else newub = floor(newub);
  }

  if (newlb > ub + eTol_ || newub < lb - eTol_) return -1;

  if (newlb > lb) { lb = newlb; tighten_lb = true; }
  if (newub < ub) { ub = newub; tighten_ub = true; }

  if (lb > ub + eTol_) return -1;
  if (!tighten_lb && !tighten_ub) return 0;

  *changed = true;
  ++pStats_.vBnd;

 if (tighten_lb && tighten_ub) {
    VarBoundMod2Ptr b2mod = (VarBoundMod2Ptr) new VarBoundMod2(v, lb, ub);
    b2mod->applyToProblem(p_);
    p_mods.push_back(b2mod);
    if (mod_rel) {
      b2mod = (VarBoundMod2Ptr) new VarBoundMod2(rel->getVariable(v->getIndex()), lb, ub);
      b2mod->applyToProblem(rel);
      r_mods.push_back(b2mod);
    }
  } else if (tighten_lb) {
    VarBoundModPtr bmod = (VarBoundModPtr) new VarBoundMod(v, Lower, lb);
    bmod->applyToProblem(p_);
    p_mods.push_back(bmod);
    if (mod_rel) {
      bmod = (VarBoundModPtr) new VarBoundMod(rel->getVariable(v->getIndex()), Lower, lb);
      bmod->applyToProblem(rel);
      r_mods.push_back(bmod);
    }
  } else if (tighten_ub) {
    VarBoundModPtr bmod = (VarBoundModPtr) new VarBoundMod(v, Upper, ub);
    bmod->applyToProblem(p_);
    p_mods.push_back(bmod);
    if (mod_rel) {
      bmod = (VarBoundModPtr) new VarBoundMod(rel->getVariable(v->getIndex()), Upper, ub);
      bmod->applyToProblem(rel);
      r_mods.push_back(bmod);
    }
  }  return 0;
}

bool PowHandler::varBndsFromCons_(bool *changed) {
  bool is_inf = false;
  for (auto it = consd_.begin(); it != consd_.end(); ++it) {
    is_inf = propPowBnds_(*it, changed);
    if (is_inf) {
      return true;
    }
  }
  return false;
}

SolveStatus PowHandler::presolve(PreModQ *, bool *changed, Solution **) {
  SolveStatus status = Started;
  ++pStats_.iters;
  *changed = false;

  if (!consd_.empty()) {
    if (varBndsFromCons_(changed)) return SolvedInfeasible;
  }

  dupRows_(changed);
  p_->delMarkedCons();
  if (status == Started) status = Finished;
  return status;
}

bool PowHandler::presolveNode(RelaxationPtr, NodePtr, SolutionPoolPtr, ModVector &, ModVector &) {
  return false;
}

std::string PowHandler::getName() const {
  return "PowHandler (Handling x^k terms).";
}

void PowHandler::writeStats(std::ostream &out) const {
  out << me_ << "Statistics for presolve by PowHandler:" << std::endl
      << me_ << "Number of iterations            = " << pStats_.iters << std::endl
      << me_ << "Number of power constraints    = " << consd_.size() << std::endl
      << me_ << "Number of constraints deleted  = " << pStats_.conDel << std::endl
      << me_ << "Times variables tightened      = " << pStats_.vBnd << std::endl;

  out << me_ << "Statistics for separation by PowHandler:" << std::endl
      << me_ << "Number of calls to separate    = " << sStats_.iters << std::endl
      << me_ << "Number of tangent cuts added   = " << sStats_.tangentcuts << std::endl;
}

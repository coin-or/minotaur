// 
//     MINOTAUR -- It's only 1/2 bull
// 
//     (C)opyright 2009 - 2014 The MINOTAUR Team.
// 

#ifndef IPOPTENGINEUT_H
#define IPOPTENGINEUT_H

#include <cppunit/TestCase.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TextTestResult.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Problem.h"
#include "Jacobian.h"
#include "HessianOfLag.h"
#include "NonlinearFunction.h"

using namespace Minotaur;

class IpoptEngineUT : public CppUnit::TestCase {
  public:
    IpoptEngineUT(std::string name) : TestCase(name) {}
    IpoptEngineUT() {}

    void testGetObjVal();
    void setUp();
    void tearDown();

    CPPUNIT_TEST_SUITE(IpoptEngineUT);
    CPPUNIT_TEST(testGetObjVal);
    CPPUNIT_TEST_SUITE_END();


  private:
    ProblemPtr instance_;
    void createInstance_();
};

// ------------------------------------------------------------------------- //
// ------------------------------------------------------------------------- //
// objective function
class myNLFun0 : public NonlinearFunction {
  public:
    myNLFun0() {}
    NonlinearFunctionPtr cloneWithVars(VariableConstIterator,
                                       Int *err) const 
    {*err = 1; return NonlinearFunctionPtr();};
    
    
    Double eval(const Double *x, Int *error);
    void evalGradient(const Double *x, Double *grad_f, Int *error);
    void evalHessian(const Double, const Double *, 
                     const LTHessStor *, Double *, 
                     Int *) {};
    void fillHessStor(LTHessStor *) {};
    void fillJac(const Double*, Double *, Int*) {};
    void finalHessStor(const LTHessStor *) {};
    UInt getGradientNzCount();
    UInt getHessianNzCount();
    void getVars(VariableSet *) {};
    bool isZero() const { return true; }
    void multiply(const Double) {};
    void prepJac(VarSetConstIter, VarSetConstIter) {};
};

// ------------------------------------------------------------------------- //
// ------------------------------------------------------------------------- //
// function in constraint 
class myNLFun1 : public NonlinearFunction {
  public:
    myNLFun1() {}
    NonlinearFunctionPtr cloneWithVars(VariableConstIterator,
                                       Int *err) const 
    {*err = 1; return NonlinearFunctionPtr();};
    
    
    Double eval(const Double *x, Int *error);
    void evalGradient(const Double *, Double *, Int *) {}
    void evalHessian(const Double, const Double *, 
                     const LTHessStor *, Double *, 
                     Int *) {};
    void fillHessStor(LTHessStor *) {};
    void fillJac(const Double*, Double *, Int*) {};
    void finalHessStor(const LTHessStor *) {};
    UInt getGradientNzCount();
    UInt getHessianNzCount();
    void getVars(VariableSet *) {};
    bool isZero() const { return true; }
    void multiply(const Double) {};
    void prepJac(VarSetConstIter, VarSetConstIter) {};
};

// ------------------------------------------------------------------------- //
// ------------------------------------------------------------------------- //
// jacobian 
class myJac : public Jacobian {
  public:
    myJac();
    UInt getNumNz();
    void fillRowColIndices(UInt *iRow, UInt *jCol);
    void fillRowColValues(const Double *x, Double *values, Int *error);


  private:
    UInt nrows_;
    UInt ncols_;

};

// ------------------------------------------------------------------------- //
// ------------------------------------------------------------------------- //
// hessian 
class myHess : public HessianOfLag {
  public:
    myHess();
    UInt getNumNz() const;
    void fillRowColIndices(UInt *iRow, UInt *jCol);
    void fillRowColValues(const Double *x, Double obj_mult, 
                          const Double *con_mult, Double *values, Int *error);

};

// ------------------------------------------------------------------------- //
// ------------------------------------------------------------------------- //

typedef boost::shared_ptr<myNLFun0> myNLFun0Ptr;
typedef boost::shared_ptr<myNLFun1> myNLFun1Ptr;
typedef boost::shared_ptr<myJac> myJacPtr;
typedef boost::shared_ptr<myHess> myHessPtr;

#endif     // #define IPOPTENGINEUT_H

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

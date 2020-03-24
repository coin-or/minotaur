#  NLP written by GAMS Convert at 01/12/18 13:32:10
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          5        5        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          6        6        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         17        7       10        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 := 0.5;
var x3 := 0.5;
var x4 := 2;
var x5 := 1;
var x6 := 1;

minimize obj: log(x2)*x2 + log(x3)*x3 - log(x4 - x6) + x4 - 0.353553390593274*
    log((x4 + 2.41421356237309*x6)/(x4 - 0.414213562373095*x6))*x5/x6 + 
    2.5746329124341*x2 + 0.54639755131421*x3 - 1;

subject to

e2: x4^3 - x4^2*(1 - x6) + (-3*x6^2 - 2*x6 + x5)*x4 - x5*x6 + x6^3 + x6^2 = 0;

e3: -(0.884831*x2*x2 + 0.555442*x2*x3 + 0.555442*x3*x2 + 0.427888*x3*x3) + x5
     = 0;

e4:  - 0.0885973*x2 - 0.0890893*x3 + x6 = 0;

e5:    x2 + x3 = 1;

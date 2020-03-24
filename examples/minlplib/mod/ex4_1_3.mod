#  NLP written by GAMS Convert at 01/12/18 13:31:55
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        1        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          2        2        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          2        1        1        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 6.325, >= 0, <= 10;

minimize obj: 8.9248e-5*x1 - 0.0218343*x1^2 + 0.998266*x1^3 - 1.6995*x1^4 + 0.2
    *x1^5;

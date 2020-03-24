#  NLP written by GAMS Convert at 01/12/18 13:41:56
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        0        0        2        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          2        2        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          4        0        4        0


var x1 := 1, >= 1, <= 5.5;
var x2 := 1, >= 1, <= 5.5;

minimize obj: x1;

subject to

e1: 0.25*x1 - 0.0625*x1^2 - 0.0625*x2^2 + 0.5*x2 <= 1;

e2: 0.0714285714285714*x1^2 + 0.0714285714285714*x2^2 - 0.428571428571429*x1 - 
    0.428571428571429*x2 <= -1;

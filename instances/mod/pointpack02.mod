#  NLP written by GAMS Convert at 01/12/18 13:38:35
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        0        0        3        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          5        5        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          9        5        4        0


var x1 := 0.5, >= 0.5, <= 1;
var x2 >= 0, <= 1;
var x3 >= 0, <= 1;
var x4 >= 0, <= 1;
var x5;

maximize obj: x5;

subject to

e1: 2*x1*x2 - x1*x1 - x2*x2 - x3*x3 + 2*x3*x4 - x4*x4 + x5 <= 0;

e2:  - x3 + x4 <= 0;

e3:  - x1 + x2 <= 0;

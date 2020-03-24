#  NLP written by GAMS Convert at 01/12/18 13:41:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        1        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          3        3        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          3        1        2        0


var x1 := 2.28067255148468E-6, >= -100, <= 100;
var x2 := 0.999139149741104, >= -2, <= 2;
var x3 := 0.998154959548312, >= -2, <= 2;

minimize obj: x1;

subject to

e1: 100*(x3 - x2^2)^2 + (1 - x2)^2 - x1 = 0;

#  MINLP written by GAMS Convert at 01/12/18 13:36:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          4        2        2        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        2        0        2        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         10        3        7        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 100, >= 0, <= 200;
var i2 integer := 100, >= 0, <= 200;
var x3 := 100, >= 0, <= 100;

minimize obj: 0.04712385*sqrt(900 + i1^2)*i2;

subject to

e1: 420.169404664517*sqrt(900 + i1^2) - x3*i1*i2 = 0;

e2:  - x3 >= -100;

e3: (2960.87631843 + 18505.4769901875*i2^2)/(7200 + i1^2) - x3 >= 0;

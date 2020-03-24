#  MINLP written by GAMS Convert at 01/12/18 13:41:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          3        1        0        2        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          3        2        0        1        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          7        5        2        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x2 >= 0, <= 10;
var i3 integer >= 0, <= 10;

minimize obj: 1.1*((-10 + 2*x2)^2 + (-5 + i3)^2) + sin((-10 + 2*x2)^2 + (-5 + 
    i3)^2);

subject to

e1:    0.7*x2 + i3 <= 7;

e2:    2.5*x2 + i3 <= 19;

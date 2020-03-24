#  MINLP written by GAMS Convert at 01/12/18 13:34:21
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        2        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        3        0        4        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          9        5        4        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 12, >= 12, <= 60;
var i2 integer := 12, >= 12, <= 60;
var i3 integer := 12, >= 12, <= 60;
var i4 integer := 12, >= 12, <= 60;
var x6 >= 0;
var x7 >= 0;

minimize obj:    x6 + x7;

subject to

e1: -1000000*i1*i2/(i3*i4) - x6 + x7 = -144279.32477276;

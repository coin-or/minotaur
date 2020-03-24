#  MINLP written by GAMS Convert at 01/12/18 13:36:58
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          4        1        3        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          4        1        0        3        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         13        1       12        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 1, >= 0, <= 200;
var i2 integer := 1, >= 0, <= 200;
var i3 integer := 1, >= 0, <= 200;

minimize obj: 7*i1^2 + 6*i2^2 - 15.8*i1 - 93.2*i2 + 8*i3^2 - 6*i3*i1 + 4*i3*i2
     - 63*i3;

subject to

e1: (-9*i1^2) - 10*i1*i2 - 8*i2^2 - 5*i3^2 - 6*i3*i1 - 10*i3*i2 >= -1000;

e2: (-6*i1^2) - 8*i1*i2 - 6*i2^2 - 4*i3^2 - 2*i3*i1 - 2*i3*i2 >= -550;

e3: (-9*i1^2) - 6*i2^2 - 8*i3^2 + 2*i2*i1 + 2*i3*i2 >= -340;

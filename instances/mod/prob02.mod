#  MINLP written by GAMS Convert at 01/12/18 13:41:56
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          9        1        1        7        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        1        0        6        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         32       22       10        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 15, >= 1, <= 100;
var i2 integer := 4, >= 1, <= 100;
var i3 integer := 2, >= 1, <= 100;
var i4 integer := 5, >= 1, <= 100;
var i5 integer := 2, >= 1, <= 100;
var i6 integer := 7, >= 1, <= 100;

minimize obj:    8000*i1 - 330*i2 - 360*i3 - 370*i4 - 415*i5 - 435*i6;

subject to

e2:    330*i2 + 360*i3 + 370*i4 + 415*i5 + 435*i6 <= 8000;

e3:    330*i2 + 360*i3 + 370*i4 + 415*i5 + 435*i6 >= 7700;

e4:    i2 + i3 + i4 + i5 + i6 <= 20;

e5: -i1*i2 <= -60;

e6: -i1*i3 <= -30;

e7: -i1*i4 <= -75;

e8: -i1*i5 <= -30;

e9: -i1*i6 <= -100;

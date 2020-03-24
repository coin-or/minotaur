#  MINLP written by GAMS Convert at 01/12/18 13:37:00
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          7        1        6        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          7        1        0        6        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         43        1       42        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 1, >= 0, <= 200;
var i2 integer := 1, >= 0, <= 200;
var i3 integer := 1, >= 0, <= 200;
var i4 integer := 1, >= 0, <= 200;
var i5 integer := 1, >= 0, <= 200;
var i6 integer := 1, >= 0, <= 200;

minimize obj: 7*i1^2 + 6*i2^2 + 0.2*i1 - 53.6*i2 + 8*i3^2 - 6*i3*i1 + 4*i3*i2
     + 4.4*i3 + 6*i4^2 + 2*i4*i1 + 2*i4*i3 - 24.8*i4 + 7*i5^2 - 4*i5*i1 - 2*i5*
    i2 - 6*i5*i3 - 104.8*i5 + 4*i6^2 + 2*i6*i1 - 4*i6*i2 - 4*i6*i3 - 2*i6*i4 + 
    6*i6*i5 - 56.4*i6;

subject to

e1: (-9*i1^2) - 10*i1*i2 - 8*i2^2 - 5*i3^2 - 6*i3*i1 - 10*i3*i2 - 7*i4^2 - 10*
    i4*i1 - 6*i4*i2 - 2*i4*i3 - 2*i5*i2 - 7*i5^2 - 6*i6*i1 - 2*i6*i2 - 2*i6*i4
     - 5*i6^2 >= -1800;

e2: (-6*i1^2) - 8*i1*i2 - 6*i2^2 - 4*i3^2 - 2*i3*i1 - 2*i3*i2 - 8*i4^2 + 2*i4*
    i1 + 10*i4*i2 - 2*i5*i1 - 6*i5*i2 + 6*i5*i4 + 7*i5^2 - 2*i6*i2 + 8*i6*i3 + 
    2*i6*i4 - 4*i6*i5 - 8*i6^2 >= -1520;

e3: (-9*i1^2) - 6*i2^2 - 8*i3^2 + 2*i2*i1 + 2*i3*i2 - 6*i4^2 + 4*i4*i1 + 4*i4*
    i2 - 2*i4*i3 - 6*i5*i1 - 2*i5*i2 + 4*i5*i4 + 6*i5^2 + 2*i6*i1 + 4*i6*i2 - 6
    *i6*i4 - 2*i6*i5 - 5*i6^2 >= -1000;

e4: (-8*i1^2) - 4*i2^2 - 9*i3^2 - 7*i4^2 - 2*i2*i1 - 2*i3*i1 - 4*i3*i2 + 6*i4*
    i1 + 2*i4*i2 - 2*i4*i3 - 6*i5*i1 - 4*i5*i2 - 2*i5*i3 + 6*i5*i4 + 6*i5^2 - 
    10*i6*i1 - 10*i6*i3 + 4*i6*i4 - 2*i6*i5 - 7*i6^2 >= -1745;

e5: 2*i2*i1 - 4*i1^2 - 5*i2^2 - 6*i3*i1 - 8*i3^2 - 2*i4*i1 + 6*i4*i2 - 2*i4*i3
     - 6*i4^2 - 4*i5*i1 + 2*i5*i2 - 6*i5*i3 - 8*i5*i4 - 7*i5^2 + 4*i6*i1 - 4*i6
    *i2 + 6*i6*i3 + 4*i6*i5 - 7*i6^2 >= -1070;

e6: 2*i2*i1 - 7*i1^2 - 7*i2^2 - 6*i3*i1 - 2*i3*i2 - 6*i3^2 - 2*i4*i1 + 2*i4*i2
     - 2*i4*i3 - 5*i4^2 - 2*i5*i1 - 4*i5*i3 + 2*i5*i4 - 5*i5^2 + 2*i6*i1 - 4*i6
    *i2 + 4*i6*i3 + 2*i6*i4 + 6*i6*i5 - 9*i6^2 >= -990;

#  MINLP written by GAMS Convert at 01/12/18 13:45:57
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         25        7        0       18        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         38        5       31        2        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        210      202        8        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var i3 integer := 1, >= 1, <= 100;
var i4 integer := 1, >= 1, <= 100;
var x5 := 1, >= 1;
var x6 := 1, >= 1;
var x7 := 1, >= 1;
var x8 := 1, >= 1;
var b9 binary >= 0, <= 1;
var b10 binary >= 0, <= 1;
var b11 binary >= 0, <= 1;
var b12 binary >= 0, <= 1;
var b13 binary >= 0, <= 1;
var b14 binary >= 0, <= 1;
var b15 binary >= 0, <= 1;
var b16 binary >= 0, <= 1;
var b17 binary >= 0, <= 1;
var b18 binary >= 0, <= 1;
var b19 binary >= 0, <= 1;
var b20 binary >= 0, <= 1;
var b21 binary >= 0, <= 1;
var b22 binary >= 0, <= 1;
var b23 binary >= 0, <= 1;
var b24 binary >= 0, <= 1;
var b25 binary >= 0, <= 1;
var b26 binary >= 0, <= 1;
var b27 binary >= 0, <= 1;
var b28 binary >= 0, <= 1;
var b29 binary >= 0, <= 1;
var b30 binary >= 0, <= 1;
var b31 binary >= 0, <= 1;
var b32 binary >= 0, <= 1;
var b33 binary >= 0, <= 1;
var b34 binary >= 0, <= 1;
var b35 binary >= 0, <= 1;
var b36 binary >= 0, <= 1;
var b37 binary >= 0, <= 1;

minimize obj:    0.1*b1 + 0.2*b2 + b9 + 2*b10 + 3*b11 + 4*b12 + 5*b13 + 6*b14
     + 7*b15 + 8*b16 + b17 + 2*b18 + 3*b19 + 4*b20 + 5*b21 + 6*b22 + 7*b23;

subject to

e2:    460*b24 + 920*b25 + 1380*b26 + 1840*b27 + 570*b32 + 1140*b33 + 1710*b34
     <= 1900;

e3:    460*b28 + 920*b29 + 1380*b30 + 1840*b31 + 570*b35 + 1140*b36 + 1710*b37
     <= 1900;

e4:  - 460*b24 - 920*b25 - 1380*b26 - 1840*b27 - 570*b32 - 1140*b33 - 1710*b34
     <= -1700;

e5:  - 460*b28 - 920*b29 - 1380*b30 - 1840*b31 - 570*b35 - 1140*b36 - 1710*b37
     <= -1700;

e6:    b24 + 2*b25 + 3*b26 + 4*b27 + b32 + 2*b33 + 3*b34 <= 5;

e7:    b28 + 2*b29 + 3*b30 + 4*b31 + b35 + 2*b36 + 3*b37 <= 5;

e8:    b1 - b9 - 2*b10 - 3*b11 - 4*b12 - 5*b13 - 6*b14 - 7*b15 - 8*b16 <= 0;

e9:    b2 - b17 - 2*b18 - 3*b19 - 4*b20 - 5*b21 - 6*b22 - 7*b23 <= 0;

e10:  - 8*b1 + b9 + 2*b10 + 3*b11 + 4*b12 + 5*b13 + 6*b14 + 7*b15 + 8*b16 <= 0;

e11:  - 7*b2 + b17 + 2*b18 + 3*b19 + 4*b20 + 5*b21 + 6*b22 + 7*b23 <= 0;

e12:    i3 - 3*b9 - 8*b10 - 15*b11 - 24*b12 - 35*b13 - 48*b14 - 63*b15 - 80*b16
      = 1;

e13:    i4 - 3*b17 - 8*b18 - 15*b19 - 24*b20 - 35*b21 - 48*b22 - 63*b23 = 1;

e14:    b9 + b10 + b11 + b12 + b13 + b14 + b15 + b16 <= 1;

e15:    b17 + b18 + b19 + b20 + b21 + b22 + b23 <= 1;

e16:    x5 - 3*b24 - 8*b25 - 15*b26 - 24*b27 = 1;

e17:    x6 - 3*b28 - 8*b29 - 15*b30 - 24*b31 = 1;

e18:    x7 - 3*b32 - 8*b33 - 15*b34 = 1;

e19:    x8 - 3*b35 - 8*b36 - 15*b37 = 1;

e20:    b24 + b25 + b26 + b27 <= 1;

e21:    b28 + b29 + b30 + b31 <= 1;

e22:    b32 + b33 + b34 <= 1;

e23:    b35 + b36 + b37 <= 1;

e24: -(sqrt(i3*x5) + sqrt(i4*x6)) + b9 + 2*b10 + 3*b11 + 4*b12 + 5*b13 + 6*b14
      + 7*b15 + 8*b16 + b17 + 2*b18 + 3*b19 + 4*b20 + 5*b21 + 6*b22 + 7*b23
      + b24 + 2*b25 + 3*b26 + 4*b27 + b28 + 2*b29 + 3*b30 + 4*b31 <= -10;

e25: -(sqrt(i3*x7) + sqrt(i4*x8)) + b9 + 2*b10 + 3*b11 + 4*b12 + 5*b13 + 6*b14
      + 7*b15 + 8*b16 + b17 + 2*b18 + 3*b19 + 4*b20 + 5*b21 + 6*b22 + 7*b23
      + b32 + 2*b33 + 3*b34 + b35 + 2*b36 + 3*b37 <= -9;

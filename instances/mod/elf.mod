#  MINLP written by GAMS Convert at 01/12/18 13:31:42
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         39        7       32        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         55       31       24        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        178      148       30        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary := 0.33, >= 0, <= 1;
var b2 binary := 0.33, >= 0, <= 1;
var b3 binary := 0.33, >= 0, <= 1;
var b4 binary := 0.33, >= 0, <= 1;
var b5 binary := 0.33, >= 0, <= 1;
var b6 binary := 0.33, >= 0, <= 1;
var b7 binary := 0.33, >= 0, <= 1;
var b8 binary := 0.33, >= 0, <= 1;
var b9 binary := 0.33, >= 0, <= 1;
var b10 binary := 0.33, >= 0, <= 1;
var b11 binary := 0.33, >= 0, <= 1;
var b12 binary := 0.33, >= 0, <= 1;
var b13 binary := 0.33, >= 0, <= 1;
var b14 binary := 0.33, >= 0, <= 1;
var b15 binary := 0.33, >= 0, <= 1;
var b16 binary := 0.33, >= 0, <= 1;
var b17 binary := 0.33, >= 0, <= 1;
var b18 binary := 0.33, >= 0, <= 1;
var b19 binary := 0.33, >= 0, <= 1;
var b20 binary := 0.33, >= 0, <= 1;
var b21 binary := 0.33, >= 0, <= 1;
var b22 binary := 0.33, >= 0, <= 1;
var b23 binary := 0.33, >= 0, <= 1;
var b24 binary >= 0, <= 1;
var x25 >= 0;
var x26 >= 0;
var x27 >= 0;
var x28 >= 0;
var x29 >= 0;
var x30 >= 0;
var x31 >= 0;
var x32 >= 0;
var x33 >= 0;
var x34 >= 0;
var x35 >= 0;
var x36 >= 0;
var x37 >= 0;
var x38 >= 0;
var x39 >= 0;
var x40 >= 0;
var x41 >= 0;
var x42 >= 0;
var x43 >= 0;
var x44 >= 0;
var x45 >= 0;
var x46 >= 0;
var x47 >= 0;
var x48 >= 0;
var x49 >= 0;
var x50 >= 0;
var x51 >= 0;
var x52 >= 0;
var x53 >= 0;
var x54 >= 0;

minimize obj:    x25 + x26 + x27 + x28 + x29 + x30 + x31 + x32 + x33 + x34
     + x35 + x36 + x37 + x38 + x39 + x40 + x41 + x42 + x43 + x44 + x45 + x46
     + x47 + x48;

subject to

e1: -(8 - x49)^2 - 100*b1 + x25 >= -100;

e2: -(8 - x50)^2 - 100*b2 + x26 >= -100;

e3: -(8 - x51)^2 - 100*b3 + x27 >= -100;

e4: -(8.5 - x49)^2 - 100*b4 + x28 >= -100;

e5: -(8.5 - x50)^2 - 100*b5 + x29 >= -100;

e6: -(8.5 - x51)^2 - 100*b6 + x30 >= -100;

e7: -(8.3 - x49)^2 - 100*b7 + x31 >= -100;

e8: -(8.3 - x50)^2 - 100*b8 + x32 >= -100;

e9: -(8.3 - x51)^2 - 100*b9 + x33 >= -100;

e10: -(8.7 - x49)^2 - 100*b10 + x34 >= -100;

e11: -(8.7 - x50)^2 - 100*b11 + x35 >= -100;

e12: -(8.7 - x51)^2 - 100*b12 + x36 >= -100;

e13: -(8.6 - x49)^2 - 100*b13 + x37 >= -100;

e14: -(8.6 - x50)^2 - 100*b14 + x38 >= -100;

e15: -(8.6 - x51)^2 - 100*b15 + x39 >= -100;

e16: -(9 - x49)^2 - 100*b16 + x40 >= -100;

e17: -(9 - x50)^2 - 100*b17 + x41 >= -100;

e18: -(9 - x51)^2 - 100*b18 + x42 >= -100;

e19: -(9.2 - x49)^2 - 100*b19 + x43 >= -100;

e20: -(9.2 - x50)^2 - 100*b20 + x44 >= -100;

e21: -(9.2 - x51)^2 - 100*b21 + x45 >= -100;

e22: -(9.5 - x49)^2 - 100*b22 + x46 >= -100;

e23: -(9.5 - x50)^2 - 100*b23 + x47 >= -100;

e24: -(9.5 - x51)^2 - 100*b24 + x48 >= -100;

e25:    b1 + b2 + b3 >= 1;

e26:    b4 + b5 + b6 >= 1;

e27:    b7 + b8 + b9 >= 1;

e28:    b10 + b11 + b12 >= 1;

e29:    b13 + b14 + b15 >= 1;

e30:    b16 + b17 + b18 >= 1;

e31:    b19 + b20 + b21 >= 1;

e32:    b22 + b23 + b24 >= 1;

e33:  - b1 - b4 - b7 - b10 - b13 - b16 - b19 - b22 + x52 = 0;

e34:  - b2 - b5 - b8 - b11 - b14 - b17 - b20 - b23 + x53 = 0;

e35:  - b3 - b6 - b9 - b12 - b15 - b18 - b21 - b24 + x54 = 0;

e36: x52*x49 - 8*b1 - 8.5*b4 - 8.3*b7 - 8.7*b10 - 8.6*b13 - 9*b16 - 9.2*b19
      - 9.5*b22 = 0;

e37: x53*x50 - 8*b2 - 8.5*b5 - 8.3*b8 - 8.7*b11 - 8.6*b14 - 9*b17 - 9.2*b20
      - 9.5*b23 = 0;

e38: x54*x51 - 8*b3 - 8.5*b6 - 8.3*b9 - 8.7*b12 - 8.6*b15 - 9*b18 - 9.2*b21
      - 9.5*b24 = 0;

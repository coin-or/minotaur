#  MINLP written by GAMS Convert at 01/12/18 13:30:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         23        1        0       22        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         43       33        0       10        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         85       64       21        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 1, >= 1, <= 10;
var i2 integer := 1, >= 1, <= 10;
var i3 integer := 1, >= 1, <= 10;
var i4 integer := 1, >= 1, <= 10;
var i5 integer := 1, >= 1, <= 10;
var i6 integer := 1, >= 1, <= 10;
var i7 integer := 1, >= 1, <= 10;
var i8 integer := 1, >= 1, <= 10;
var i9 integer := 1, >= 1, <= 10;
var i10 integer := 1, >= 1, <= 10;
var x11 := 1, >= 1, <= 10;
var x12 := 1, >= 1, <= 10;
var x13 := 1, >= 1, <= 10;
var x14 := 1, >= 1, <= 10;
var x15 := 1, >= 1, <= 10;
var x16 := 1, >= 1, <= 10;
var x17 := 1, >= 1, <= 10;
var x18 := 1, >= 1, <= 10;
var x19 := 1, >= 1, <= 10;
var x20 := 1, >= 1, <= 10;
var x21 := 1E-8, >= 1E-8;
var x22 <= 0;
var x23 <= 0;
var x24 <= 0;
var x25 <= 0;
var x26 <= 0;
var x27 <= 0;
var x28 <= 0;
var x29 <= 0;
var x30 <= 0;
var x31 <= 0;
var x32 <= 0;
var x33 <= 0;
var x34 <= 0;
var x35 <= 0;
var x36 <= 0;
var x37 <= 0;
var x38 <= 0;
var x39 <= 0;
var x40 <= 0;
var x41 <= 0;
var x42;

minimize obj:    i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 + x11 + x12
     + x13 + x14 + x15 + x16 + x17 + x18 + x19 + x20 + 20000*x21;

subject to

e2:    x22 + x23 + x24 + x25 + x26 + x27 + x28 + x29 + x30 + x31 + x32 + x33
     + x34 + x35 + x36 + x37 + x38 + x39 + x40 + x41 + x42 <= 0;

e3: -0.32*log(i1) - x22 <= 0;

e4: -0.19*log(i2) - x23 <= 0;

e5: -0.405*log(i3) - x24 <= 0;

e6: -0.265*log(i4) - x25 <= 0;

e7: -0.175*log(i5) - x26 <= 0;

e8: -0.44*log(i6) - x27 <= 0;

e9: -0.275*log(i7) - x28 <= 0;

e10: -0.47*log(i8) - x29 <= 0;

e11: -0.31*log(i9) - x30 <= 0;

e12: -0.295*log(i10) - x31 <= 0;

e13: -0.105*log(x11) - x32 <= 0;

e14: -0.15*log(x12) - x33 <= 0;

e15: -0.235*log(x13) - x34 <= 0;

e16: -0.115*log(x14) - x35 <= 0;

e17: -0.42*log(x15) - x36 <= 0;

e18: -0.095*log(x16) - x37 <= 0;

e19: -0.115*log(x17) - x38 <= 0;

e20: -0.085*log(x18) - x39 <= 0;

e21: -0.115*log(x19) - x40 <= 0;

e22: -0.022*log(x20) - x41 <= 0;

e23: -log(x21) - x42 <= 0;

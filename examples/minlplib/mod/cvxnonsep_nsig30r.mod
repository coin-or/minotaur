#  MINLP written by GAMS Convert at 01/12/18 13:30:49
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         32        1        0       31        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         61       46        0       15        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        121       91       30        0
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
var i11 integer := 1, >= 1, <= 10;
var i12 integer := 1, >= 1, <= 10;
var i13 integer := 1, >= 1, <= 10;
var i14 integer := 1, >= 1, <= 10;
var i15 integer := 1, >= 1, <= 10;
var x16 := 1E-5, >= 1E-5, <= 10;
var x17 := 1E-5, >= 1E-5, <= 10;
var x18 := 1E-5, >= 1E-5, <= 10;
var x19 := 1E-5, >= 1E-5, <= 10;
var x20 := 1E-5, >= 1E-5, <= 10;
var x21 := 1E-5, >= 1E-5, <= 10;
var x22 := 1E-5, >= 1E-5, <= 10;
var x23 := 1E-5, >= 1E-5, <= 10;
var x24 := 1E-5, >= 1E-5, <= 10;
var x25 := 1E-5, >= 1E-5, <= 10;
var x26 := 1E-5, >= 1E-5, <= 10;
var x27 := 1E-5, >= 1E-5, <= 10;
var x28 := 1E-5, >= 1E-5, <= 10;
var x29 := 1E-5, >= 1E-5, <= 10;
var x30 := 1E-5, >= 1E-5, <= 10;
var x32;
var x33;
var x34;
var x35;
var x36;
var x37;
var x38;
var x39;
var x40;
var x41;
var x42;
var x43;
var x44;
var x45;
var x46;
var x47;
var x48;
var x49;
var x50;
var x51;
var x52;
var x53;
var x54;
var x55;
var x56;
var x57;
var x58;
var x59;
var x60;
var x61;

minimize obj:    1.29*i1 + 0.76*i2 + i3 + 1.62*i4 + 1.07*i5 + 0.7*i6 + 1.88*i7
     + 1.75*i8 + 1.1*i9 + 1.24*i10 + 1.17*i11 + 0.82*i12 + 1.06*i13 + 0.94*i14
     + 0.46*i15 + 1.69*x16 + 0.39*x17 + 0.45*x18 + 0.34*x19 + 0.46*x20
     + 1.86*x21 + 1.46*x22 + 0.98*x23 + 1.16*x24 + 0.47*x25 + 0.92*x26
     + 1.43*x27 + 1.81*x28 + 1.78*x29 + 0.67*x30;

subject to

e2:    x32 + x33 + x34 + x35 + x36 + x37 + x38 + x39 + x40 + x41 + x42 + x43
     + x44 + x45 + x46 + x47 + x48 + x49 + x50 + x51 + x52 + x53 + x54 + x55
     + x56 + x57 + x58 + x59 + x60 + x61 <= -1.77195684190188;

e3: -0.028*log(i1) - x32 <= 0;

e4: -0.041*log(i2) - x33 <= 0;

e5: -0.047*log(i3) - x34 <= 0;

e6: -0.022*log(i4) - x35 <= 0;

e7: -0.041*log(i5) - x36 <= 0;

e8: -0.026*log(i6) - x37 <= 0;

e9: -0.052*log(i7) - x38 <= 0;

e10: -0.051*log(i8) - x39 <= 0;

e11: -0.016*log(i9) - x40 <= 0;

e12: -0.038*log(i10) - x41 <= 0;

e13: -0.036*log(i11) - x42 <= 0;

e14: -0.033*log(i12) - x43 <= 0;

e15: -0.054*log(i13) - x44 <= 0;

e16: -0.016*log(i14) - x45 <= 0;

e17: -0.02*log(i15) - x46 <= 0;

e18: -0.007*log(x16) - x47 <= 0;

e19: -0.058*log(x17) - x48 <= 0;

e20: -0.04*log(x18) - x49 <= 0;

e21: -0.03*log(x19) - x50 <= 0;

e22: -0.039*log(x20) - x51 <= 0;

e23: -0.034*log(x21) - x52 <= 0;

e24: -0.04*log(x22) - x53 <= 0;

e25: -0.034*log(x23) - x54 <= 0;

e26: -0.044*log(x24) - x55 <= 0;

e27: -0.032*log(x25) - x56 <= 0;

e28: -0.061*log(x26) - x57 <= 0;

e29: -0.013*log(x27) - x58 <= 0;

e30: -0.007*log(x28) - x59 <= 0;

e31: -0.007*log(x29) - x60 <= 0;

e32: -0.004*log(x30) - x61 <= 0;

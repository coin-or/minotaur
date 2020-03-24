#  MINLP written by GAMS Convert at 01/12/18 13:30:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         31        1        0       30        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         60       45        0       15        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        147       89       58        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer >= 0, <= 5;
var i2 integer >= 0, <= 5;
var i3 integer >= 0, <= 5;
var i4 integer >= 0, <= 5;
var i5 integer >= 0, <= 5;
var i6 integer >= 0, <= 5;
var i7 integer >= 0, <= 5;
var i8 integer >= 0, <= 5;
var i9 integer >= 0, <= 5;
var i10 integer >= 0, <= 5;
var i11 integer >= 0, <= 5;
var i12 integer >= 0, <= 5;
var i13 integer >= 0, <= 5;
var i14 integer >= 0, <= 5;
var i15 integer >= 0, <= 5;
var x16 >= 0, <= 5;
var x17 >= 0, <= 5;
var x18 >= 0, <= 5;
var x19 >= 0, <= 5;
var x20 >= 0, <= 5;
var x21 >= 0, <= 5;
var x22 >= 0, <= 5;
var x23 >= 0, <= 5;
var x24 >= 0, <= 5;
var x25 >= 0, <= 5;
var x26 >= 0, <= 5;
var x27 >= 0, <= 5;
var x28 >= 0, <= 5;
var x29 >= 0, <= 5;
var x30 >= 0, <= 5;
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

minimize obj:  - 0.52*i1 - 0.94*i2 - 0.64*i3 - 0.96*i4 - 0.24*i5 - 0.68*i6
     - 0.29*i7 - 0.67*i8 - 0.7*i9 - 0.07*i10 - 0.25*i11 - 0.22*i12 - 0.67*i13
     - 0.84*i14 - 0.34*i15 - 0.78*x16 - 0.68*x17 - 0.01*x18 - 0.6*x19
     - 0.39*x20 - 0.92*x21 - 0.1*x22 - 0.46*x23 - 0.77*x24 - 0.32*x25
     - 0.78*x26 - 0.37*x27 - 0.78*x28 - 0.47*x29 - 0.04*x30;

subject to

e2:    x32 + x33 + x34 + x35 + x36 + x37 + x38 + x39 + x40 + x41 + x42 + x43
     + x44 + x45 + x46 + x47 + x48 + x49 + x50 + x51 + x52 + x53 + x54 + x55
     + x56 + x57 + x58 + x59 + x60 <= 360;

e3: 2**(i1 + i2) - x32 <= 0;

e4: 2**(i2 + i3) - x33 <= 0;

e5: 2**(i3 + i4) - x34 <= 0;

e6: 2**(i4 + i5) - x35 <= 0;

e7: 2**(i5 + i6) - x36 <= 0;

e8: 2**(i6 + i7) - x37 <= 0;

e9: 2**(i7 + i8) - x38 <= 0;

e10: 2**(i8 + i9) - x39 <= 0;

e11: 2**(i9 + i10) - x40 <= 0;

e12: 2**(i10 + i11) - x41 <= 0;

e13: 2**(i11 + i12) - x42 <= 0;

e14: 2**(i12 + i13) - x43 <= 0;

e15: 2**(i13 + i14) - x44 <= 0;

e16: 2**(i14 + i15) - x45 <= 0;

e17: 2**(i15 + x16) - x46 <= 0;

e18: 2**(x16 + x17) - x47 <= 0;

e19: 2**(x17 + x18) - x48 <= 0;

e20: 2**(x18 + x19) - x49 <= 0;

e21: 2**(x19 + x20) - x50 <= 0;

e22: 2**(x20 + x21) - x51 <= 0;

e23: 2**(x21 + x22) - x52 <= 0;

e24: 2**(x22 + x23) - x53 <= 0;

e25: 2**(x23 + x24) - x54 <= 0;

e26: 2**(x24 + x25) - x55 <= 0;

e27: 2**(x25 + x26) - x56 <= 0;

e28: 2**(x26 + x27) - x57 <= 0;

e29: 2**(x27 + x28) - x58 <= 0;

e30: 2**(x28 + x29) - x59 <= 0;

e31: 2**(x29 + x30) - x60 <= 0;

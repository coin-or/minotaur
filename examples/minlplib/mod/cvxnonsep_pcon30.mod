#  MINLP written by GAMS Convert at 01/12/18 13:30:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         31       16        0       15        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         61       31       30        0
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

minimize obj:  - 0.52*i1 - 0.94*i2 - 0.64*i3 - 0.96*i4 - 0.24*i5 - 0.68*i6
     - 0.29*i7 - 0.67*i8 - 0.7*i9 - 0.07*i10 - 0.25*i11 - 0.22*i12 - 0.67*i13
     - 0.84*i14 - 0.34*i15 - 0.78*x16 - 0.68*x17 - 0.01*x18 - 0.6*x19
     - 0.39*x20 - 0.92*x21 - 0.1*x22 - 0.46*x23 - 0.77*x24 - 0.32*x25
     - 0.78*x26 - 0.37*x27 - 0.78*x28 - 0.47*x29 - 0.04*x30;

subject to

e2: (2**(i1 + i2) + 2**(i2 + i3) + 2**(i3 + i4) + 2**(i4 + i5) + 2**(i5 + i6)
     + 2**(i6 + i7) + 2**(i7 + i8) + 2**(i8 + i9) + 2**(i9 + i10) + 2**(i10 + 
    i11) + 2**(i11 + i12) + 2**(i12 + i13) + 2**(i13 + i14) + 2**(i14 + i15) + 
    2**(i15 + x16) + 2**(x16 + x17) + 2**(x17 + x18) + 2**(x18 + x19) + 2**(x19
     + x20) + 2**(x20 + x21) + 2**(x21 + x22) + 2**(x22 + x23) + 2**(x23 + x24)
     + 2**(x24 + x25) + 2**(x25 + x26) + 2**(x26 + x27) + 2**(x27 + x28) + 2**(
    x28 + x29) + 2**(x29 + x30))^2 <= 129600;

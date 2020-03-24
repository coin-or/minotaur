#  MINLP written by GAMS Convert at 01/12/18 13:30:49
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         22        1        0       21        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         41       31        0       10        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         81       61       20        0
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
var x11 := 1E-5, >= 1E-5, <= 10;
var x12 := 1E-5, >= 1E-5, <= 10;
var x13 := 1E-5, >= 1E-5, <= 10;
var x14 := 1E-5, >= 1E-5, <= 10;
var x15 := 1E-5, >= 1E-5, <= 10;
var x16 := 1E-5, >= 1E-5, <= 10;
var x17 := 1E-5, >= 1E-5, <= 10;
var x18 := 1E-5, >= 1E-5, <= 10;
var x19 := 1E-5, >= 1E-5, <= 10;
var x20 := 1E-5, >= 1E-5, <= 10;
var x21;
var x22;
var x23;
var x24;
var x25;
var x26;
var x27;
var x28;
var x29;
var x30;
var x31;
var x32;
var x33;
var x34;
var x35;
var x36;
var x37;
var x38;
var x39;
var x40;

minimize obj:    1.5*i1 + 0.51*i2 + 1.01*i3 + 1.4*i4 + 1.78*i5 + 1.92*i6
     + 1.09*i7 + 0.48*i8 + 0.67*i9 + 0.52*i10 + 1.68*x11 + 0.51*x12 + 1.63*x13
     + 0.49*x14 + 1.86*x15 + 0.7*x16 + 0.39*x17 + 0.5*x18 + 1.23*x19 + 0.95*x20
    ;

subject to

e2:    x21 + x22 + x23 + x24 + x25 + x26 + x27 + x28 + x29 + x30 + x31 + x32
     + x33 + x34 + x35 + x36 + x37 + x38 + x39 + x40 <= -1.6094379124341;

e3: -0.065*log(i1) - x21 <= 0;

e4: -0.004*log(i2) - x22 <= 0;

e5: -0.084*log(i3) - x23 <= 0;

e6: -0.093*log(i4) - x24 <= 0;

e7: -0.06*log(i5) - x25 <= 0;

e8: -0.075*log(i6) - x26 <= 0;

e9: -0.074*log(i7) - x27 <= 0;

e10: -0.039*log(i8) - x28 <= 0;

e11: -0.065*log(i9) - x29 <= 0;

e12: -0.017*log(i10) - x30 <= 0;

e13: -0.07*log(x11) - x31 <= 0;

e14: -0.03*log(x12) - x32 <= 0;

e15: -0.028*log(x13) - x33 <= 0;

e16: -0.005*log(x14) - x34 <= 0;

e17: -0.01*log(x15) - x35 <= 0;

e18: -0.082*log(x16) - x36 <= 0;

e19: -0.069*log(x17) - x37 <= 0;

e20: -0.032*log(x18) - x38 <= 0;

e21: -0.095*log(x19) - x39 <= 0;

e22: -0.003*log(x20) - x40 <= 0;

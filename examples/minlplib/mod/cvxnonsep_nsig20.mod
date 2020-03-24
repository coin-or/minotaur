#  MINLP written by GAMS Convert at 01/12/18 13:30:49
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         21       11        0       10        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         41       21       20        0
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

minimize obj:    1.5*i1 + 0.51*i2 + 1.01*i3 + 1.4*i4 + 1.78*i5 + 1.92*i6
     + 1.09*i7 + 0.48*i8 + 0.67*i9 + 0.52*i10 + 1.68*x11 + 0.51*x12 + 1.63*x13
     + 0.49*x14 + 1.86*x15 + 0.7*x16 + 0.39*x17 + 0.5*x18 + 1.23*x19 + 0.95*x20
    ;

subject to

e2: -0.2*i1**0.065*i2**0.004*i3**0.084*i4**0.093*i5**0.06*i6**0.075*i7**0.074*
    i8**0.039*i9**0.065*i10**0.017*x11**0.07*x12**0.03*x13**0.028*x14**0.005*
    x15**0.01*x16**0.082*x17**0.069*x18**0.032*x19**0.095*x20**0.003 <= -1;

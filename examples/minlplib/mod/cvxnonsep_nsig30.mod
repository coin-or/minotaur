#  MINLP written by GAMS Convert at 01/12/18 13:30:49
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

minimize obj:    1.29*i1 + 0.76*i2 + i3 + 1.62*i4 + 1.07*i5 + 0.7*i6 + 1.88*i7
     + 1.75*i8 + 1.1*i9 + 1.24*i10 + 1.17*i11 + 0.82*i12 + 1.06*i13 + 0.94*i14
     + 0.46*i15 + 1.69*x16 + 0.39*x17 + 0.45*x18 + 0.34*x19 + 0.46*x20
     + 1.86*x21 + 1.46*x22 + 0.98*x23 + 1.16*x24 + 0.47*x25 + 0.92*x26
     + 1.43*x27 + 1.81*x28 + 1.78*x29 + 0.67*x30;

subject to

e2: -0.2*i1**0.028*i2**0.041*i3**0.047*i4**0.022*i5**0.041*i6**0.026*i7**0.052*
    i8**0.051*i9**0.016*i10**0.038*i11**0.036*i12**0.033*i13**0.054*i14**0.016*
    i15**0.02*x16**0.007*x17**0.058*x18**0.04*x19**0.03*x20**0.039*x21**0.034*
    x22**0.04*x23**0.034*x24**0.044*x25**0.032*x26**0.061*x27**0.013*x28**0.007
    *x29**0.007*x30**0.004 <= -1;

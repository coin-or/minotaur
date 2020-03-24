#  MINLP written by GAMS Convert at 01/12/18 13:30:49
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         41       21        0       20        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         81       41       40        0
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
var i16 integer := 1, >= 1, <= 10;
var i17 integer := 1, >= 1, <= 10;
var i18 integer := 1, >= 1, <= 10;
var i19 integer := 1, >= 1, <= 10;
var i20 integer := 1, >= 1, <= 10;
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
var x31 := 1E-5, >= 1E-5, <= 10;
var x32 := 1E-5, >= 1E-5, <= 10;
var x33 := 1E-5, >= 1E-5, <= 10;
var x34 := 1E-5, >= 1E-5, <= 10;
var x35 := 1E-5, >= 1E-5, <= 10;
var x36 := 1E-5, >= 1E-5, <= 10;
var x37 := 1E-5, >= 1E-5, <= 10;
var x38 := 1E-5, >= 1E-5, <= 10;
var x39 := 1E-5, >= 1E-5, <= 10;
var x40 := 1E-5, >= 1E-5, <= 10;

minimize obj:    1.1*i1 + 1.86*i2 + 0.86*i3 + 1.23*i4 + 0.72*i5 + 0.47*i6
     + 0.98*i7 + 0.39*i8 + 0.25*i9 + 0.41*i10 + 0.29*i11 + 0.38*i12 + 1.49*i13
     + 1.27*i14 + 0.56*i15 + 1.08*i16 + 1.39*i17 + i18 + 1.37*i19 + 0.89*i20
     + 0.25*x21 + 0.98*x22 + 1.71*x23 + 1.75*x24 + 0.54*x25 + 0.42*x26
     + 1.13*x27 + 1.28*x28 + 0.83*x29 + 0.41*x30 + 1.9*x31 + 0.16*x32
     + 0.21*x33 + 0.28*x34 + 0.33*x35 + 1.24*x36 + 1.15*x37 + 0.1*x38
     + 1.86*x39 + 1.46*x40;

subject to

e2: -0.2*i1**0.035*i2**0.003*i3**0.04*i4**0.044*i5**0.046*i6**0.04*i7**0.037*i8
    **0.024*i9**0.008*i10**0.019*i11**0.006*i12**0.001*i13**0.044*i14**0.014*
    i15**0.014*i16**0.016*i17**0.022*i18**0.03*i19**0.001*i20**0.039*x21**0.026
    *x22**0.04*x23**0.016*x24**0.021*x25**0.003*x26**0.008*x27**0.031*x28**
    0.015*x29**0.042*x30**0.006*x31**0.046*x32**0.025*x33**0.033*x34**0.047*x35
    **0.013*x36**0.019*x37**0.022*x38**0.036*x39**0.038*x40**0.005 <= -1;

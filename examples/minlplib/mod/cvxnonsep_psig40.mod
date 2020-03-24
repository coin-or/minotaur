#  MINLP written by GAMS Convert at 01/12/18 13:30:51
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        1        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         41       21        0       20        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         41        1       40        0
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
var x21 := 1, >= 1, <= 10;
var x22 := 1, >= 1, <= 10;
var x23 := 1, >= 1, <= 10;
var x24 := 1, >= 1, <= 10;
var x25 := 1, >= 1, <= 10;
var x26 := 1, >= 1, <= 10;
var x27 := 1, >= 1, <= 10;
var x28 := 1, >= 1, <= 10;
var x29 := 1, >= 1, <= 10;
var x30 := 1, >= 1, <= 10;
var x31 := 1, >= 1, <= 10;
var x32 := 1, >= 1, <= 10;
var x33 := 1, >= 1, <= 10;
var x34 := 1, >= 1, <= 10;
var x35 := 1, >= 1, <= 10;
var x36 := 1, >= 1, <= 10;
var x37 := 1, >= 1, <= 10;
var x38 := 1, >= 1, <= 10;
var x39 := 1, >= 1, <= 10;
var x40 := 1, >= 1, <= 10;

minimize obj: 40000*i1**(-0.015)*i2**(-0.37)*i3**(-0.25)*i4**(-0.24)*i5**(-0.45
    )*i6**(-0.305)*i7**(-0.31)*i8**(-0.43)*i9**(-0.405)*i10**(-0.29)*i11**(-
    0.09)*i12**(-0.12)*i13**(-0.445)*i14**(-0.015)*i15**(-0.245)*i16**(-0.085)*
    i17**(-0.49)*i18**(-0.355)*i19**(-0.25)*i20**(-0.235)*x21**(-0.03)*x22**(-
    0.34)*x23**(-0.02)*x24**(-0.035)*x25**(-0.26)*x26**(-0.05)*x27**(-0.41)*x28
    **(-0.41)*x29**(-0.36)*x30**(-0.075)*x30**(-0.36)*x31**(-0.33)*x32**(-0.26)
    *x33**(-0.485)*x34**(-0.325)*x35**(-0.4)*x36**(-0.225)*x37**(-0.215)*x38**(
    -0.415)*x39**(-0.04)*x40**(-0.065) + i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8
     + i9 + i10 + i11 + i12 + i13 + i14 + i15 + i16 + i17 + i18 + i19 + i20 + 
    x21 + x22 + x23 + x24 + x25 + x26 + x27 + x28 + x29 + x30 + x31 + x32 + x33
     + x34 + x35 + x36 + x37 + x38 + x39 + x40;

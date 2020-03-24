#  MINLP written by GAMS Convert at 01/12/18 13:30:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        1        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         31       16        0       15        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         31        1       30        0
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
var x16 := 1, >= 1, <= 10;
var x17 := 1, >= 1, <= 10;
var x18 := 1, >= 1, <= 10;
var x19 := 1, >= 1, <= 10;
var x20 := 1, >= 1, <= 10;
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

minimize obj: 30000*i1**(-0.48)*i2**(-0.275)*i3**(-0.26)*i4**(-0.215)*i5**(-
    0.245)*i6**(-0.31)*i7**(-0.34)*i8**(-0.2)*i9**(-0.185)*i10**(-0.495)*i11**(
    -0.02)*i12**(-0.445)*i13**(-0.455)*i14**(-0.4)*i15**(-0.05)*x16**(-0.13)*
    x17**(-0.17)*x18**(-0.34)*x19**(-0.07)*x20**(-0.36)*x21**(-0.05)*x22**(-
    0.325)*x23**(-0.245)*x24**(-0.39)*x25**(-0.36)*x26**(-0.45)*x27**(-0.445)*
    x28**(-0.165)*x29**(-0.35)*x30**(-0.1) + i1 + i2 + i3 + i4 + i5 + i6 + i7
     + i8 + i9 + i10 + i11 + i12 + i13 + i14 + i15 + x16 + x17 + x18 + x19 + 
    x20 + x21 + x22 + x23 + x24 + x25 + x26 + x27 + x28 + x29 + x30;

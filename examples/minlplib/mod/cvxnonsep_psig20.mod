#  MINLP written by GAMS Convert at 01/12/18 13:30:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        1        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         21       11        0       10        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         21        1       20        0
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

minimize obj: 20000*i1**(-0.32)*i2**(-0.19)*i3**(-0.405)*i4**(-0.265)*i5**(-
    0.175)*i6**(-0.44)*i7**(-0.275)*i8**(-0.47)*i9**(-0.31)*i10**(-0.295)*x11**
    (-0.105)*x12**(-0.15)*x13**(-0.235)*x14**(-0.115)*x15**(-0.42)*x16**(-0.095
    )*x17**(-0.115)*x18**(-0.085)*x19**(-0.115)*x20**(-0.22) + i1 + i2 + i3 + 
    i4 + i5 + i6 + i7 + i8 + i9 + i10 + x11 + x12 + x13 + x14 + x15 + x16 + x17
     + x18 + x19 + x20;

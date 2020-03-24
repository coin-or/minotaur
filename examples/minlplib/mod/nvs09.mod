#  MINLP written by GAMS Convert at 01/12/18 13:36:58
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        1        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         11        1        0       10        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         11        1       10        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer := 5, >= 3, <= 9;
var i2 integer := 5, >= 3, <= 9;
var i3 integer := 5, >= 3, <= 9;
var i4 integer := 5, >= 3, <= 9;
var i5 integer := 5, >= 3, <= 9;
var i6 integer := 5, >= 3, <= 9;
var i7 integer := 5, >= 3, <= 9;
var i8 integer := 5, >= 3, <= 9;
var i9 integer := 5, >= 3, <= 9;
var i10 integer := 5, >= 3, <= 9;

minimize obj: log((-2) + i1)^2 + log(10 - i1)^2 + log((-2) + i2)^2 + log(10 - 
    i2)^2 + log((-2) + i3)^2 + log(10 - i3)^2 + log((-2) + i4)^2 + log(10 - i4)
    ^2 + log((-2) + i5)^2 + log(10 - i5)^2 + log((-2) + i6)^2 + log(10 - i6)^2
     + log((-2) + i7)^2 + log(10 - i7)^2 + log((-2) + i8)^2 + log(10 - i8)^2 + 
    log((-2) + i9)^2 + log(10 - i9)^2 + log((-2) + i10)^2 + log(10 - i10)^2 - (
    i1*i2*i3*i4*i5*i6*i7*i8*i9*i10)**0.2;

#  MINLP written by GAMS Convert at 01/12/18 13:29:55
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        1        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         11        1        0       10        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         21       11       10        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i2 integer >= -1, <= 2;
var i3 integer >= -1, <= 2;
var i4 integer >= -1, <= 2;
var i5 integer >= -1, <= 2;
var i6 integer >= -1, <= 2;
var i7 integer >= -1, <= 2;
var i8 integer >= -1, <= 2;
var i9 integer >= -1, <= 2;
var i10 integer >= -1, <= 2;
var i11 integer >= -1, <= 2;

minimize obj:  - i2 - i3 - i4 - i5 - i6 - i7 - i8 - i9 - i10 - i11;

subject to

e2: 0.116545517321418*i10^2 - 0.116545517321418*i10 + 0.048698282657444*i9^2 - 
    0.048698282657444*i9 + 0.167136633802493*i8^2 - 0.167136633802493*i8 + 
    0.172842180379538*i7^2 - 0.172842180379538*i7 + 0.0350835273588374*i6^2 - 
    0.0350835273588374*i6 + 0.133517550184507*i5^2 - 0.133517550184507*i5 + 
    0.107213563760389*i4^2 - 0.107213563760389*i4 + 0.0605518448846168*i3^2 - 
    0.0605518448846168*i3 + 0.0745422678604453*i2^2 - 0.0745422678604453*i2 + 
    0.0838686317903121*i11^2 - 0.0838686317903121*i11 <= -9.9999999999989E-5;

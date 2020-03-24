#  MINLP written by GAMS Convert at 01/12/18 13:29:54
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


var i2 integer >= -1, <= 1;
var i3 integer >= -1, <= 1;
var i4 integer >= -1, <= 1;
var i5 integer >= -1, <= 1;
var i6 integer >= -1, <= 1;
var i7 integer >= -1, <= 1;
var i8 integer >= -1, <= 1;
var i9 integer >= -1, <= 1;
var i10 integer >= -1, <= 1;
var i11 integer >= -1, <= 1;

minimize obj:  - i2 - i3 - i4 - i5 - i6 - i7 - i8 - i9 - i10 - i11;

subject to

e2: i10^2 - 0.987420882906575*i10 + i9^2 - 0.987420882906575*i9 + i8^2 - 
    0.987420882906575*i8 + i7^2 - 0.987420882906575*i7 + i6^2 - 
    0.987420882906575*i6 + i5^2 - 0.987420882906575*i5 + i4^2 - 
    0.987420882906575*i4 + i3^2 - 0.987420882906575*i3 + i2^2 - 
    0.987420882906575*i2 + i11^2 - 0.987420882906575*i11 <= 0;

#  MINLP written by GAMS Convert at 02/12/18 09:52:52
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


var i2 integer >= -100;
var i3 integer >= -100;
var i4 integer >= -100;
var i5 integer >= -100;
var i6 integer >= -100;
var i7 integer >= -100;
var i8 integer >= -100;
var i9 integer >= -100;
var i10 integer >= -100;
var i11 integer >= -100;

minimize obj:    9*i2 + 8*i3 + 7*i4 + 6*i5 + 5*i6 + 4*i7 + 3*i8 + 2*i9 + i10
     + 10*i11;

subject to

e2: 100*i10^2 - 98*i10 + 100*i9^2 - 98*i9 + 100*i8^2 - 98*i8 + 100*i7^2 - 98*i7
     + 100*i6^2 - 98*i6 + 100*i5^2 - 98*i5 + 100*i4^2 - 98*i4 + 100*i3^2 - 98*
    i3 + 100*i2^2 - 98*i2 + 100*i11^2 - 98*i11 - 2*i10*i9 - 2*i10*i9 - 2*i8*i7
     - 2*i8*i7 - 2*i6*i5 - 2*i6*i5 - 2*i4*i3 - 2*i4*i3 - 2*i2*i11 - 2*i2*i11
     <= -1;

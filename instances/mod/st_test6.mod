#  MINLP written by GAMS Convert at 01/12/18 13:43:36
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          6        1        0        5        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         11        1        0       10        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         57       47       10        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer >= 0, <= 1;
var i2 integer >= 0, <= 1;
var i3 integer >= 0, <= 1;
var i4 integer >= 0, <= 1;
var i5 integer >= 0, <= 1;
var i6 integer >= 0, <= 1;
var i7 integer >= 0, <= 1;
var i8 integer >= 0, <= 1;
var i9 integer >= 0, <= 1;
var i10 integer >= 0, <= 1;

minimize obj: 50*i1*i1 + 48*i1 + 50*i2*i2 + 42*i2 + 50*i3*i3 + 48*i3 + 50*i4*i4
     + 45*i4 + 50*i5*i5 + 44*i5 + 50*i6*i6 + 41*i6 + 50*i7*i7 + 47*i7 + 50*i8*
    i8 + 42*i8 + 50*i9*i9 + 45*i9 + 50*i10*i10 + 46*i10;

subject to

e1:  - 2*i1 - 6*i2 - i3 - 3*i5 - 3*i6 - 2*i7 - 6*i8 - 2*i9 - 2*i10 <= -4;

e2:    6*i1 - 5*i2 + 8*i3 - 3*i4 + i6 + 3*i7 + 8*i8 + 9*i9 - 3*i10 <= 22;

e3:  - 5*i1 + 6*i2 + 5*i3 + 3*i4 + 8*i5 - 8*i6 + 9*i7 + 2*i8 - 9*i10 <= -6;

e4:    9*i1 + 5*i2 - 9*i4 + i5 - 8*i6 + 3*i7 - 9*i8 - 9*i9 - 3*i10 <= -23;

e5:  - 8*i1 + 7*i2 - 4*i3 - 5*i4 - 9*i5 + i6 - 7*i7 - i8 + 3*i9 - 2*i10 <= -12;

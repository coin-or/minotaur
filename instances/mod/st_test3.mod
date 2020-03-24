#  MINLP written by GAMS Convert at 01/12/18 13:43:35
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         11        1        0       10        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         14        1        0       13        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         42       37        5        0
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
var i10 integer >= 0, <= 1E15;
var i11 integer >= 0, <= 1E15;
var i12 integer >= 0, <= 1E15;
var i13 integer >= 0, <= 1;

minimize obj: 5*i1*i1 + 5*i1 + 5*i2*i2 - 5*i2 + 5*i3*i3 + 5*i3 + 5*i4*i4 + 5*i4
     + 9*i10*i10 - i10 - i5 + i6 - i7 - i8 - i9 - i11 - i12 - i13;

subject to

e1:    2*i1 + 2*i2 + i10 + i11 <= 10;

e2:    2*i1 + 2*i3 + i10 + i12 <= 10;

e3:    2*i2 + 2*i3 + i11 + i12 <= 10;

e4:  - 8*i1 + i10 <= 0;

e5:  - 8*i2 + i11 <= 0;

e6:  - 8*i3 + i12 <= 0;

e7:  - 2*i4 - i5 + i10 <= 0;

e8:  - 2*i6 - i7 + i11 <= 0;

e9:  - 2*i8 - i9 + i12 <= 0;

e10:    i13 <= 1;

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
#         52       42       10        0
# 
#  Reformulation has removed 1 variable and 1 equation


var i1 integer >= 0, <= 100;
var i2 integer >= 0, <= 100;
var i3 integer >= 0, <= 100;
var i4 integer >= 0, <= 100;
var i5 integer >= 0, <= 100;
var i6 integer >= 0, <= 100;
var i7 integer >= 0, <= 100;
var i8 integer >= 0, <= 100;
var i9 integer >= 0, <= 100;
var i10 integer >= 0, <= 100;

minimize obj: 0.00055*i1*i1 - 0.0583*i1 + 0.0019*i2*i2 + 0.2318*i2 + 0.0002*i3*
    i3 - 0.0108*i3 + 0.00095*i4*i4 + 0.1634*i4 + 0.0046*i5*i5 - 0.138*i5 + 
    0.0035*i6*i6 + 0.357*i6 + 0.00315*i7*i7 - 0.1953*i7 + 0.00475*i8*i8 - 0.361
    *i8 + 0.0048*i9*i9 + 0.1824*i9 + 0.003*i10*i10 - 0.162*i10;

subject to

e1:    8*i1 + 7*i2 + 9*i3 + 9*i5 + 8*i6 + 2*i7 + 4*i9 + i10 <= 530;

e2:    3*i1 + 4*i2 + 6*i3 + 9*i4 + 6*i6 + 9*i7 + i8 + i10 <= 395;

e3:    2*i2 + i3 + 5*i4 + 5*i5 + 7*i7 + 4*i8 + 2*i9 <= 350;

e4:    5*i1 + 7*i3 + i4 + 7*i5 + 5*i6 + 7*i8 + 9*i9 + 5*i10 <= 405;

e5:    i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 <= 200;

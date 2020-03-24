#  NLP written by GAMS Convert at 01/12/18 13:43:34
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          6        1        0        5        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         11       11        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         52       42       10        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;
var x4 >= 0;
var x5 >= 0;
var x6 >= 0;
var x7 >= 0;
var x8 >= 0;
var x9 >= 0;
var x10 >= 0;

minimize obj: (-0.00055*x1^2) - 0.0583*x1 - 0.0019*x2^2 - 0.2318*x2 - 0.0002*x3
    ^2 - 0.0108*x3 - 0.00095*x4^2 - 0.1634*x4 - 0.0046*x5^2 - 0.138*x5 - 0.0035
    *x6^2 - 0.357*x6 - 0.00315*x7^2 - 0.1953*x7 - 0.00475*x8^2 - 0.361*x8 - 
    0.0048*x9^2 - 0.1824*x9 - 0.003*x10^2 - 0.162*x10;

subject to

e1:    8*x1 + 7*x2 + 9*x3 + 9*x5 + 8*x6 + 2*x7 + 4*x9 + x10 <= 530;

e2:    3*x1 + 4*x2 + 6*x3 + 9*x4 + 6*x6 + 9*x7 + x8 + x10 <= 395;

e3:    2*x2 + x3 + 5*x4 + 5*x5 + 7*x7 + 4*x8 + 2*x9 <= 350;

e4:    5*x1 + 7*x3 + x4 + 7*x5 + 5*x6 + 7*x8 + 9*x9 + 5*x10 <= 405;

e5:    x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 <= 200;

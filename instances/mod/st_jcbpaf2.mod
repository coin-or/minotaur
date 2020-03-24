#  NLP written by GAMS Convert at 01/12/18 13:43:29
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         14        1        2       11        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         11       11        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        116      106       10        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 100;
var x2 >= 0, <= 100;
var x3 >= 0, <= 100;
var x4 >= 0, <= 100;
var x5 >= 0, <= 100;
var x6 >= 0, <= 100;
var x7 >= 0, <= 100;
var x8 >= 0, <= 100;
var x9 >= 0, <= 100;
var x10 >= 0, <= 100;

minimize obj: x1*x6 - x1 - x6 + x2*x7 - 2*x2 - 2*x7 + x3*x8 - 3*x3 - 3*x8 + x4*
    x9 - 4*x4 - 4*x9 + x5*x10 - 5*x5 - 5*x10;

subject to

e1:    x1 + 7*x2 + 5*x3 + 5*x4 - 6*x6 - 3*x7 - 3*x8 + 5*x9 - 7*x10 <= 80;

e2:  - 3*x1 + 3*x2 + 8*x3 + 7*x4 - 9*x5 - 7*x6 - 9*x7 + 8*x9 - 7*x10 <= 57;

e3:    x1 + x3 + 3*x4 + 8*x5 + 9*x6 + 9*x8 - 7*x9 - 8*x10 <= 92;

e4:  - x1 - 2*x2 + 2*x3 + 9*x5 + 5*x6 - 3*x7 + x8 - x9 - 5*x10 <= 55;

e5:  - 5*x1 + 8*x2 - 8*x3 + 3*x5 + 4*x7 - 5*x8 - 2*x9 + 9*x10 <= 76;

e6:    4*x1 - x2 + 6*x3 - 4*x4 - 7*x5 - 8*x6 - 7*x7 + 6*x8 - 2*x9 - 9*x10
     <= 14;

e7:    7*x2 + 4*x3 + 9*x5 - 6*x8 - 5*x9 - 5*x10 <= 47;

e8:  - 5*x1 - x2 + 7*x4 - x5 + 2*x6 + 5*x7 - 8*x8 - 5*x9 + 2*x10 <= 51;

e9:  - 4*x1 - 7*x2 - 9*x4 + 2*x5 + 6*x6 - 9*x7 + x8 - 5*x9 <= 36;

e10:  - 2*x1 + 6*x2 + 8*x4 - 6*x5 + 8*x6 + 8*x7 + 5*x8 + 2*x9 - 7*x10 <= 92;

e11:    x1 + x2 + x3 - 2*x4 + x5 + x6 + x7 + 4*x8 + x9 + 3*x10 <= 200;

e12:    x1 + x2 + x3 + x4 + x5 >= 1;

e13:    x6 + x7 + x8 + x9 + x10 >= 2;

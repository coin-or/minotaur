#  NLP written by GAMS Convert at 01/12/18 13:43:29
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          8        1        7        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          9        9        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         23       15        8        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= -1, <= 1;
var x2 >= -2.1, <= 2;
var x3 >= -3.2, <= 3;
var x4 >= -4.3, <= 4;
var x5 >= -5.4, <= 5;
var x6 >= -6.5, <= 6;
var x7 >= -7.6, <= 7;
var x8 >= -8.7, <= 8;

minimize obj: 0.845*x1*x1 + 7*x1 + 0.5*x1*x2 + 6*x2 + x1*x3 + 5*x3 + 1.5*x1*x4
     + 4*x4 + 2*x1*x5 + 3*x5 + 2.5*x1*x6 + 2*x6 + 3*x1*x7 + x7 + 3.5*x1*x8 + 
    0.5*x2*x1 + 0.845*x2*x2 + 0.5*x2*x3 + x2*x4 + 1.5*x2*x5 + 2*x2*x6 + 2.5*x2*
    x7 + 3*x2*x8 + x3*x1 + 0.5*x3*x2 + 0.845*x3*x3 + 0.5*x3*x4 + x3*x5 + 1.5*x3
    *x6 + 2*x3*x7 + 2.5*x3*x8 + 1.5*x4*x1 + x4*x2 + 0.5*x4*x3 + 0.845*x4*x4 + 
    0.5*x4*x5 + x4*x6 + 1.5*x4*x7 + 2*x4*x8 + 2*x5*x1 + 1.5*x5*x2 + x5*x3 + 0.5
    *x5*x4 + 0.845*x5*x5 + 0.5*x5*x6 + x5*x7 + 1.5*x5*x8 + 2.5*x6*x1 + 2*x6*x2
     + 1.5*x6*x3 + x6*x4 + 0.5*x6*x5 + 0.845*x6*x6 + 0.5*x6*x7 + x6*x8 + 3*x7*
    x1 + 2.5*x7*x2 + 2*x7*x3 + 1.5*x7*x4 + x7*x5 + 0.5*x7*x6 + 0.845*x7*x7 + 
    0.5*x7*x8 + 3.5*x8*x1 + 3*x8*x2 + 2.5*x8*x3 + 2*x8*x4 + 1.5*x8*x5 + x8*x6
     + 0.5*x8*x7 + 0.845*x8*x8;

subject to

e1:  - x1 + x2 >= -1;

e2:  - x2 + x3 >= -1.05;

e3:  - x3 + x4 >= -1.1;

e4:  - x4 + x5 >= -1.15;

e5:  - x5 + x6 >= -1.2;

e6:  - x6 + x7 >= -1.25;

e7:  - x7 + x8 >= -1.3;

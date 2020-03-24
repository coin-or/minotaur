#  NLP written by GAMS Convert at 01/12/18 13:14:01
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         12        4        8        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         11       11        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         38       24       14        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 1745, >= 1E-6, <= 2000;
var x2 := 12000, >= 1E-6, <= 16000;
var x3 := 110, >= 1E-6, <= 120;
var x4 := 3048, >= 1E-6, <= 5000;
var x5 := 1974, >= 1E-6, <= 2000;
var x6 := 89.2, >= 85, <= 93;
var x7 := 92.8, >= 90, <= 95;
var x8 := 8, >= 3, <= 12;
var x9 := 3.6, >= 0.01, <= 4;
var x10 := 145, >= 145, <= 162;

maximize obj: 0.063*x4*x7 - 5.04*x1 - 0.035*x2 - 10*x3 - 3.36*x5;

subject to

e2:    x1 - 1.22*x4 + x5 = 0;

e3: -98000*x3/(x4*x9 + 1000*x3) + x6 = 0;

e4: -(x2 + x5)/x1 + x8 = 0;

e5: (1.12 + 0.13167*x8 - 0.00667*x8**2)*x1 - 0.99*x4 >= 0;

e6: -(1.12 + 0.13167*x8 - 0.00667*x8**2)*x1 + 1.01010101010101*x4 >= 0;

e7: 1.098*x8 - 0.038*x8**2 + 0.325*x6 - 0.99*x7 >= -57.425;

e8: -(1.098*x8 - 0.038*x8**2) - 0.325*x6 + 1.01010101010101*x7 >= 57.425;

e9:  - 0.9*x9 - 0.222*x10 >= -35.82;

e10:    1.11111111111111*x9 + 0.222*x10 >= 35.82;

e11:    3*x7 - 0.99*x10 >= 133;

e12:  - 3*x7 + 1.01010101010101*x10 >= -133;

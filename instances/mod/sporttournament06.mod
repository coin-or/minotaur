#  MINLP written by GAMS Convert at 01/12/18 13:42:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        0        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         16        1       15        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         16        1       15        0


var b1 binary >= 0, <= 1;
var b2 binary >= 0, <= 1;
var b3 binary >= 0, <= 1;
var b4 binary >= 0, <= 1;
var b5 binary >= 0, <= 1;
var b6 binary >= 0, <= 1;
var b7 binary >= 0, <= 1;
var b8 binary >= 0, <= 1;
var b9 binary >= 0, <= 1;
var b10 binary >= 0, <= 1;
var b11 binary >= 0, <= 1;
var b12 binary >= 0, <= 1;
var b13 binary >= 0, <= 1;
var b14 binary >= 0, <= 1;
var b15 binary >= 0, <= 1;
var x16;

maximize obj: x16;

subject to

e1: 2*b1*b3 - 2*b1 + 2*b3 + 2*b1*b7 - 2*b7 + 2*b2*b5 - 2*b2 - 2*b5 + 2*b2*b10
     - 4*b10 - 2*b3*b4 + 2*b4 - 2*b3*b12 - 2*b3*b14 - 2*b4*b5 + 2*b4*b9 - 2*b9
     - 2*b4*b15 + 2*b5*b6 - 2*b6 + 2*b5*b8 - 2*b8 + 2*b6*b9 - 2*b7*b8 + 2*b7*
    b12 + 2*b7*b13 + 2*b8*b10 + 2*b8*b15 + 2*b9*b11 - 2*b11 - 2*b9*b12 + 2*b10*
    b11 + 2*b10*b12 - 2*b13*b15 + 2*b14*b15 + x16 <= 0;

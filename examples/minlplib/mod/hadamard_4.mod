#  MINLP written by GAMS Convert at 12/08/18 18:03:34
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        0        1        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         17        1       16        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         17        1       16        0


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
var b16 binary >= 0, <= 1;
var x17;

maximize obj: x17;

subject to

e1: b1*b6*b11*b16 - b1*b6*b12*b15 + b1*b8*b10*b15 - b4*b5*b10*b15 + b4*b5*b11*
    b14 - b1*b8*b11*b14 + b1*b7*b12*b14 - b1*b7*b10*b16 + b3*b5*b10*b16 - b3*b5
    *b12*b14 + b3*b8*b9*b14 - b4*b7*b9*b14 + b4*b7*b10*b13 - b3*b8*b10*b13 + b3
    *b6*b12*b13 - b3*b6*b9*b16 + b2*b7*b9*b16 - b2*b7*b12*b13 + b2*b8*b11*b13
     - b4*b6*b11*b13 + b4*b6*b9*b15 - b2*b8*b9*b15 + b2*b5*b12*b15 - b2*b5*b11*
    b16 - x17 >= 0;

#  MINLP written by GAMS Convert at 01/12/18 13:42:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        0        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         29        1       28        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         29        1       28        0


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
var b17 binary >= 0, <= 1;
var b18 binary >= 0, <= 1;
var b19 binary >= 0, <= 1;
var b20 binary >= 0, <= 1;
var b21 binary >= 0, <= 1;
var b22 binary >= 0, <= 1;
var b23 binary >= 0, <= 1;
var b24 binary >= 0, <= 1;
var b25 binary >= 0, <= 1;
var b26 binary >= 0, <= 1;
var b27 binary >= 0, <= 1;
var b28 binary >= 0, <= 1;
var x29;

maximize obj: x29;

subject to

e1: 2*b1*b10 - 4*b1 - 2*b10 + 2*b1*b17 + 2*b1*b18 + 2*b1*b20 + 2*b2*b4 - 2*b2
     - 2*b4 + 2*b2*b14 - 4*b14 + 2*b3*b19 - 2*b3 + 2*b3*b21 + 2*b3*b23 - 2*b3*
    b24 + 2*b4*b5 - 2*b5 + 2*b4*b7 - 2*b7 - 2*b4*b25 + 2*b5*b8 - 4*b8 + 2*b6*b7
     + 2*b6 - 2*b6*b23 - 2*b6*b25 - 2*b6*b28 + 2*b7*b13 - 2*b13 - 2*b7*b18 + 2*
    b8*b9 - 2*b9 + 2*b8*b12 - 2*b12 + 2*b8*b25 + 2*b9*b13 + 2*b10*b24 + 2*b10*
    b27 - 2*b10*b28 + 2*b11*b12 - 2*b11 - 2*b11*b19 + 2*b11*b25 + 2*b11*b28 + 2
    *b12*b14 - 2*b12*b17 + 2*b13*b15 - 2*b15 - 2*b13*b16 + 2*b14*b15 + 2*b14*
    b16 + 2*b16*b17 - 2*b16*b18 - 2*b17*b19 + 2*b18*b23 + 2*b19*b20 - 2*b20*b21
     - 2*b20*b22 + 2*b22*b26 - 2*b23*b26 - 2*b26*b27 + 2*b26*b28 + x29 <= 0;

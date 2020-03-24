#  MINLP written by GAMS Convert at 01/12/18 13:15:03
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        0        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         21        1       20        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         21        1       20        0


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
var x21;

minimize obj: x21;

subject to

e1: 8*b1*b3 - 4*b1 - 8*b3 + 8*b2*b4 - 4*b2 - 8*b4 + 8*b3*b5 - 8*b5 + 8*b4*b6 - 
    8*b6 + 8*b5*b7 - 8*b7 + 8*b6*b8 - 8*b8 + 8*b7*b9 - 8*b9 + 8*b8*b10 - 8*b10
     + 8*b9*b11 - 8*b11 + 8*b10*b12 - 8*b12 + 8*b11*b13 - 8*b13 + 8*b12*b14 - 8
    *b14 + 8*b13*b15 - 8*b15 + 8*b14*b16 - 8*b16 + 8*b15*b17 - 8*b17 + 8*b16*
    b18 - 8*b18 + 8*b17*b19 - 4*b19 + 8*b18*b20 - 4*b20 - x21 <= 0;

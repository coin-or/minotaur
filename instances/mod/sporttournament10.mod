#  MINLP written by GAMS Convert at 01/12/18 13:42:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        0        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         46        1       45        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         46        1       45        0


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
var b29 binary >= 0, <= 1;
var b30 binary >= 0, <= 1;
var b31 binary >= 0, <= 1;
var b32 binary >= 0, <= 1;
var b33 binary >= 0, <= 1;
var b34 binary >= 0, <= 1;
var b35 binary >= 0, <= 1;
var b36 binary >= 0, <= 1;
var b37 binary >= 0, <= 1;
var b38 binary >= 0, <= 1;
var b39 binary >= 0, <= 1;
var b40 binary >= 0, <= 1;
var b41 binary >= 0, <= 1;
var b42 binary >= 0, <= 1;
var b43 binary >= 0, <= 1;
var b44 binary >= 0, <= 1;
var b45 binary >= 0, <= 1;
var x46;

maximize obj: x46;

subject to

e1: 2*b1*b14 - 2*b1 - 4*b14 + 2*b1*b34 + 2*b1*b35 - 2*b1*b39 + 2*b2*b3 - 2*b2
     - 2*b3 + 2*b2*b31 - 4*b31 + 2*b3*b4 - 2*b4 + 2*b3*b8 - 4*b8 - 2*b3*b43 + 2
    *b4*b9 - 4*b9 + 2*b5*b7 - 2*b5 - 4*b7 + 2*b5*b11 - 2*b11 + 2*b6*b7 - 2*b6
     + 2*b6*b11 + 2*b7*b13 - 2*b13 + 2*b7*b42 + 2*b8*b19 - 4*b19 + 2*b8*b28 - 2
    *b28 + 2*b8*b44 + 2*b9*b10 - 2*b10 + 2*b9*b18 - 2*b18 + 2*b9*b43 + 2*b10*
    b19 + 2*b11*b36 - 2*b11*b40 - 2*b12*b13 + 2*b12 - 2*b12*b38 + 2*b12*b40 - 2
    *b12*b45 + 2*b13*b14 + 2*b13*b25 - 2*b25 + 2*b14*b16 - 2*b16 + 2*b14*b40 - 
    2*b15*b17 - 2*b15 - 2*b17 + 2*b15*b25 + 2*b15*b35 + 2*b15*b41 + 2*b16*b17
     + 2*b16*b26 - 2*b26 - 2*b16*b44 + 2*b17*b18 + 2*b17*b43 + 2*b18*b30 - 2*
    b30 - 2*b18*b35 + 2*b19*b20 - 2*b20 + 2*b19*b29 - 2*b29 + 2*b20*b30 - 2*b21
    *b22 + 2*b21 + 2*b22 - 2*b21*b23 - 2*b23 - 2*b22*b24 - 2*b24 - 2*b22*b36 + 
    2*b22*b38 + 2*b23*b24 + 2*b23*b42 + 2*b23*b45 + 2*b24*b26 + 2*b24*b39 - 2*
    b25*b27 - 2*b27 + 2*b25*b36 + 2*b26*b27 - 2*b26*b42 + 2*b27*b28 + 2*b27*b44
     + 2*b28*b29 - 2*b28*b37 + 2*b29*b31 - 2*b29*b34 + 2*b30*b32 - 2*b32 - 2*
    b30*b33 + 2*b31*b32 + 2*b31*b33 + 2*b33*b34 - 2*b33*b35 - 2*b34*b37 - 2*b36
    *b41 + 2*b37*b39 + 2*b37*b41 - 2*b39*b40 - 2*b41*b42 - 2*b43*b44 + x46
     <= 0;

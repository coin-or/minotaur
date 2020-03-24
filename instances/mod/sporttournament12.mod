#  MINLP written by GAMS Convert at 01/12/18 13:42:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        0        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         67        1       66        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         67        1       66        0


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
var b46 binary >= 0, <= 1;
var b47 binary >= 0, <= 1;
var b48 binary >= 0, <= 1;
var b49 binary >= 0, <= 1;
var b50 binary >= 0, <= 1;
var b51 binary >= 0, <= 1;
var b52 binary >= 0, <= 1;
var b53 binary >= 0, <= 1;
var b54 binary >= 0, <= 1;
var b55 binary >= 0, <= 1;
var b56 binary >= 0, <= 1;
var b57 binary >= 0, <= 1;
var b58 binary >= 0, <= 1;
var b59 binary >= 0, <= 1;
var b60 binary >= 0, <= 1;
var b61 binary >= 0, <= 1;
var b62 binary >= 0, <= 1;
var b63 binary >= 0, <= 1;
var b64 binary >= 0, <= 1;
var b65 binary >= 0, <= 1;
var b66 binary >= 0, <= 1;
var x67;

maximize obj: x67;

subject to

e1: 2*b1*b16 - 2*b1 - 4*b16 + 2*b1*b48 + 2*b1*b49 - 2*b1*b54 + 2*b2*b4 - 2*b2
     - 2*b4 + 2*b2*b45 - 4*b45 + 2*b3*b8 - 2*b3 - 4*b8 + 2*b3*b38 - 2*b38 - 2*
    b3*b50 + 2*b3*b60 + 2*b4*b5 - 2*b5 + 2*b4*b10 - 4*b10 - 2*b4*b61 + 2*b5*b11
     - 4*b11 + 2*b6*b7 - 2*b6 - 2*b7 + 2*b6*b13 - 2*b13 + 2*b7*b8 + 2*b7*b22 - 
    2*b22 - 2*b7*b63 + 2*b8*b9 - 4*b9 + 2*b8*b52 + 2*b9*b25 + 2*b25 + 2*b9*b36
     - 2*b36 + 2*b9*b50 + 2*b10*b20 - 4*b20 + 2*b10*b30 - 2*b30 + 2*b10*b62 + 2
    *b11*b12 - 2*b12 + 2*b11*b19 - 4*b19 + 2*b11*b61 + 2*b12*b20 + 2*b13*b36 + 
    2*b13*b55 - 2*b13*b58 + 2*b14*b36 + 2*b14 - 2*b14*b55 - 2*b14*b60 - 2*b14*
    b64 + 2*b15*b16 + 2*b15 - 2*b15*b23 - 2*b23 - 2*b15*b56 - 2*b15*b65 + 2*b16
    *b17 - 2*b17 + 2*b16*b38 + 2*b17*b18 - 4*b18 + 2*b17*b27 - 2*b27 - 2*b17*
    b62 + 2*b18*b19 + 2*b18*b41 - 2*b41 + 2*b18*b61 + 2*b19*b32 - 4*b32 + 2*b19
    *b42 - 2*b42 + 2*b20*b21 - 2*b21 + 2*b20*b31 - 2*b31 + 2*b21*b32 + 2*b22*
    b23 + 2*b22*b24 - 2*b24 - 2*b22*b57 + 2*b23*b26 - 2*b26 + 2*b23*b58 + 2*b24
    *b26 + 2*b24*b38 - 2*b24*b66 - 2*b25*b27 - 2*b25*b37 - 2*b37 - 2*b25*b59 + 
    2*b26*b27 - 2*b26*b54 + 2*b27*b29 - 4*b29 - 2*b28*b30 - 2*b28 + 2*b28*b49
     + 2*b28*b56 + 2*b28*b59 + 2*b29*b30 + 2*b29*b40 - 2*b40 + 2*b29*b62 + 2*
    b30*b31 + 2*b31*b44 - 2*b44 - 2*b31*b49 + 2*b32*b33 - 2*b33 + 2*b32*b43 - 2
    *b43 + 2*b33*b44 - 2*b34*b52 + 2*b34 - 2*b34*b57 - 2*b35*b53 + 2*b35 - 2*
    b35*b58 + 2*b35*b64 - 2*b35*b66 - 2*b36*b65 + 2*b37*b39 - 4*b39 + 2*b37*b58
     + 2*b37*b60 - 2*b38*b40 + 2*b39*b40 + 2*b39*b54 + 2*b39*b65 + 2*b40*b41 + 
    2*b41*b42 - 2*b41*b59 + 2*b42*b43 - 2*b42*b51 + 2*b43*b45 - 2*b43*b48 + 2*
    b44*b46 - 2*b46 - 2*b44*b47 + 2*b45*b46 + 2*b45*b47 + 2*b47*b48 - 2*b47*b49
     - 2*b48*b51 - 2*b50*b56 + 2*b50*b59 + 2*b51*b54 + 2*b51*b56 + 2*b52*b53 - 
    2*b52*b60 + 2*b57*b63 + 2*b57*b66 - 2*b61*b62 + 2*b65*b66 + x67 <= 0;

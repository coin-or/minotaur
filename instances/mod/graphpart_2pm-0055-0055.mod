#  MINLP written by GAMS Convert at 01/12/18 13:34:27
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         26       26        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         76        1       75        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        151       76       75        0
# 
#  Reformulation has removed 1 variable and 1 equation


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
var b67 binary >= 0, <= 1;
var b68 binary >= 0, <= 1;
var b69 binary >= 0, <= 1;
var b70 binary >= 0, <= 1;
var b71 binary >= 0, <= 1;
var b72 binary >= 0, <= 1;
var b73 binary >= 0, <= 1;
var b74 binary >= 0, <= 1;
var b75 binary >= 0, <= 1;

minimize obj: b1*b4 - b1*b13 + b1*b16 - b1*b61 + b2*b5 - b2*b14 + b2*b17 - b2*
    b62 + b3*b6 - b3*b15 + b3*b18 - b3*b63 - b4*b7 + b4*b19 - b4*b64 - b5*b8 + 
    b5*b20 - b5*b65 - b6*b9 + b6*b21 - b6*b66 + b7*b10 - b7*b22 - b7*b67 + b8*
    b11 - b8*b23 - b8*b68 + b9*b12 - b9*b24 - b9*b69 + b10*b13 + b10*b25 - b10*
    b70 + b11*b14 + b11*b26 - b11*b71 + b12*b15 + b12*b27 - b12*b72 - b13*b28
     - b13*b73 - b14*b29 - b14*b74 - b15*b30 - b15*b75 - b16*b19 + b16*b28 + 
    b16*b31 - b17*b20 + b17*b29 + b17*b32 - b18*b21 + b18*b30 + b18*b33 + b19*
    b22 + b19*b34 + b20*b23 + b20*b35 + b21*b24 + b21*b36 - b22*b25 - b22*b37
     - b23*b26 - b23*b38 - b24*b27 - b24*b39 + b25*b28 - b25*b40 + b26*b29 - 
    b26*b41 + b27*b30 - b27*b42 - b28*b43 - b29*b44 - b30*b45 + b31*b34 + b31*
    b43 - b31*b46 + b32*b35 + b32*b44 - b32*b47 + b33*b36 + b33*b45 - b33*b48
     - b34*b37 + b34*b49 - b35*b38 + b35*b50 - b36*b39 + b36*b51 - b37*b40 - 
    b37*b52 - b38*b41 - b38*b53 - b39*b42 - b39*b54 + b40*b43 - b40*b55 + b41*
    b44 - b41*b56 + b42*b45 - b42*b57 + b43*b58 + b44*b59 + b45*b60 - b46*b49
     + b46*b58 - b46*b61 - b47*b50 + b47*b59 - b47*b62 - b48*b51 + b48*b60 - 
    b48*b63 + b49*b52 + b49*b64 + b50*b53 + b50*b65 + b51*b54 + b51*b66 + b52*
    b55 - b52*b67 + b53*b56 - b53*b68 + b54*b57 - b54*b69 - b55*b58 - b55*b70
     - b56*b59 - b56*b71 - b57*b60 - b57*b72 + b58*b73 + b59*b74 + b60*b75 + 
    b61*b64 + b61*b73 + b62*b65 + b62*b74 + b63*b66 + b63*b75 + b64*b67 + b65*
    b68 + b66*b69 - b67*b70 - b68*b71 - b69*b72 + b70*b73 + b71*b74 + b72*b75;

subject to

e1:    b1 + b2 + b3 = 1;

e2:    b4 + b5 + b6 = 1;

e3:    b7 + b8 + b9 = 1;

e4:    b10 + b11 + b12 = 1;

e5:    b13 + b14 + b15 = 1;

e6:    b16 + b17 + b18 = 1;

e7:    b19 + b20 + b21 = 1;

e8:    b22 + b23 + b24 = 1;

e9:    b25 + b26 + b27 = 1;

e10:    b28 + b29 + b30 = 1;

e11:    b31 + b32 + b33 = 1;

e12:    b34 + b35 + b36 = 1;

e13:    b37 + b38 + b39 = 1;

e14:    b40 + b41 + b42 = 1;

e15:    b43 + b44 + b45 = 1;

e16:    b46 + b47 + b48 = 1;

e17:    b49 + b50 + b51 = 1;

e18:    b52 + b53 + b54 = 1;

e19:    b55 + b56 + b57 = 1;

e20:    b58 + b59 + b60 = 1;

e21:    b61 + b62 + b63 = 1;

e22:    b64 + b65 + b66 = 1;

e23:    b67 + b68 + b69 = 1;

e24:    b70 + b71 + b72 = 1;

e25:    b73 + b74 + b75 = 1;

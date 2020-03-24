#  MINLP written by GAMS Convert at 01/12/18 13:34:29
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         28       28        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         82        1       81        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        163       82       81        0
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
var b76 binary >= 0, <= 1;
var b77 binary >= 0, <= 1;
var b78 binary >= 0, <= 1;
var b79 binary >= 0, <= 1;
var b80 binary >= 0, <= 1;
var b81 binary >= 0, <= 1;

minimize obj: (-b1*b4) - b1*b7 + b1*b10 + b1*b19 - b1*b28 - b1*b55 - b2*b5 - b2
    *b8 + b2*b11 + b2*b20 - b2*b29 - b2*b56 - b3*b6 - b3*b9 + b3*b12 + b3*b21
     - b3*b30 - b3*b57 - b4*b7 - b4*b13 + b4*b22 - b4*b31 - b4*b58 - b5*b8 - b5
    *b14 + b5*b23 - b5*b32 - b5*b59 - b6*b9 - b6*b15 + b6*b24 - b6*b33 - b6*b60
     + b7*b16 + b7*b25 + b7*b34 - b7*b61 + b8*b17 + b8*b26 + b8*b35 - b8*b62 + 
    b9*b18 + b9*b27 + b9*b36 - b9*b63 - b10*b13 + b10*b16 - b10*b19 + b10*b37
     - b10*b64 - b11*b14 + b11*b17 - b11*b20 + b11*b38 - b11*b65 - b12*b15 + 
    b12*b18 - b12*b21 + b12*b39 - b12*b66 + b13*b16 - b13*b22 - b13*b40 + b13*
    b67 + b14*b17 - b14*b23 - b14*b41 + b14*b68 + b15*b18 - b15*b24 - b15*b42
     + b15*b69 + b16*b25 + b16*b43 + b16*b70 + b17*b26 + b17*b44 + b17*b71 + 
    b18*b27 + b18*b45 + b18*b72 - b19*b22 - b19*b25 - b19*b46 + b19*b73 - b20*
    b23 - b20*b26 - b20*b47 + b20*b74 - b21*b24 - b21*b27 - b21*b48 + b21*b75
     + b22*b25 + b22*b49 + b22*b76 + b23*b26 + b23*b50 + b23*b77 + b24*b27 + 
    b24*b51 + b24*b78 - b25*b52 - b25*b79 - b26*b53 - b26*b80 - b27*b54 - b27*
    b81 + b28*b31 - b28*b34 - b28*b37 - b28*b46 - b28*b55 + b29*b32 - b29*b35
     - b29*b38 - b29*b47 - b29*b56 + b30*b33 - b30*b36 - b30*b39 - b30*b48 - 
    b30*b57 - b31*b34 - b31*b40 - b31*b49 + b31*b58 - b32*b35 - b32*b41 - b32*
    b50 + b32*b59 - b33*b36 - b33*b42 - b33*b51 + b33*b60 - b34*b43 + b34*b52
     + b34*b61 - b35*b44 + b35*b53 + b35*b62 - b36*b45 + b36*b54 + b36*b63 + 
    b37*b40 - b37*b43 - b37*b46 + b37*b64 + b38*b41 - b38*b44 - b38*b47 + b38*
    b65 + b39*b42 - b39*b45 - b39*b48 + b39*b66 + b40*b43 + b40*b49 - b40*b67
     + b41*b44 + b41*b50 - b41*b68 + b42*b45 + b42*b51 - b42*b69 - b43*b52 + 
    b43*b70 - b44*b53 + b44*b71 - b45*b54 + b45*b72 + b46*b49 + b46*b52 + b46*
    b73 + b47*b50 + b47*b53 + b47*b74 + b48*b51 + b48*b54 + b48*b75 + b49*b52
     + b49*b76 + b50*b53 + b50*b77 + b51*b54 + b51*b78 - b52*b79 - b53*b80 - 
    b54*b81 + b55*b58 + b55*b61 - b55*b64 + b55*b73 + b56*b59 + b56*b62 - b56*
    b65 + b56*b74 + b57*b60 + b57*b63 - b57*b66 + b57*b75 - b58*b61 - b58*b67
     + b58*b76 - b59*b62 - b59*b68 + b59*b77 - b60*b63 - b60*b69 + b60*b78 + 
    b61*b70 - b61*b79 + b62*b71 - b62*b80 + b63*b72 - b63*b81 - b64*b67 - b64*
    b70 + b64*b73 - b65*b68 - b65*b71 + b65*b74 - b66*b69 - b66*b72 + b66*b75
     + b67*b70 - b67*b76 + b68*b71 - b68*b77 + b69*b72 - b69*b78 - b70*b79 - 
    b71*b80 - b72*b81 + b73*b76 + b73*b79 + b74*b77 + b74*b80 + b75*b78 + b75*
    b81 + b76*b79 + b77*b80 + b78*b81;

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

e26:    b76 + b77 + b78 = 1;

e27:    b79 + b80 + b81 = 1;

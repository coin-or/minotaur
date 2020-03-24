#  MINLP written by GAMS Convert at 01/12/18 13:34:29
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         33       33        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         97        1       96        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        193       97       96        0
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
var b82 binary >= 0, <= 1;
var b83 binary >= 0, <= 1;
var b84 binary >= 0, <= 1;
var b85 binary >= 0, <= 1;
var b86 binary >= 0, <= 1;
var b87 binary >= 0, <= 1;
var b88 binary >= 0, <= 1;
var b89 binary >= 0, <= 1;
var b90 binary >= 0, <= 1;
var b91 binary >= 0, <= 1;
var b92 binary >= 0, <= 1;
var b93 binary >= 0, <= 1;
var b94 binary >= 0, <= 1;
var b95 binary >= 0, <= 1;
var b96 binary >= 0, <= 1;

minimize obj: b1*b4 + b1*b10 - b1*b13 + b1*b25 - b1*b73 + b2*b5 + b2*b11 - b2*
    b14 + b2*b26 - b2*b74 + b3*b6 + b3*b12 - b3*b15 + b3*b27 - b3*b75 - b4*b7
     - b4*b16 + b4*b28 - b4*b76 - b5*b8 - b5*b17 + b5*b29 - b5*b77 - b6*b9 - b6
    *b18 + b6*b30 - b6*b78 - b7*b10 - b7*b19 - b7*b31 + b7*b79 - b8*b11 - b8*
    b20 - b8*b32 + b8*b80 - b9*b12 - b9*b21 - b9*b33 + b9*b81 + b10*b22 + b10*
    b34 + b10*b82 + b11*b23 + b11*b35 + b11*b83 + b12*b24 + b12*b36 + b12*b84
     - b13*b16 + b13*b22 + b13*b37 - b13*b85 - b14*b17 + b14*b23 + b14*b38 - 
    b14*b86 - b15*b18 + b15*b24 + b15*b39 - b15*b87 - b16*b19 + b16*b40 + b16*
    b88 - b17*b20 + b17*b41 + b17*b89 - b18*b21 + b18*b42 + b18*b90 - b19*b22
     + b19*b43 - b19*b91 - b20*b23 + b20*b44 - b20*b92 - b21*b24 + b21*b45 - 
    b21*b93 + b22*b46 + b22*b94 + b23*b47 + b23*b95 + b24*b48 + b24*b96 + b25*
    b28 - b25*b34 + b25*b37 - b25*b49 + b26*b29 - b26*b35 + b26*b38 - b26*b50
     + b27*b30 - b27*b36 + b27*b39 - b27*b51 - b28*b31 + b28*b40 + b28*b52 - 
    b29*b32 + b29*b41 + b29*b53 - b30*b33 + b30*b42 + b30*b54 + b31*b34 + b31*
    b43 - b31*b55 + b32*b35 + b32*b44 - b32*b56 + b33*b36 + b33*b45 - b33*b57
     - b34*b46 - b34*b58 - b35*b47 - b35*b59 - b36*b48 - b36*b60 - b37*b40 + 
    b37*b46 - b37*b61 - b38*b41 + b38*b47 - b38*b62 - b39*b42 + b39*b48 - b39*
    b63 - b40*b43 - b40*b64 - b41*b44 - b41*b65 - b42*b45 - b42*b66 - b43*b46
     - b43*b67 - b44*b47 - b44*b68 - b45*b48 - b45*b69 + b46*b70 + b47*b71 + 
    b48*b72 + b49*b52 + b49*b58 + b49*b61 - b49*b73 + b50*b53 + b50*b59 + b50*
    b62 - b50*b74 + b51*b54 + b51*b60 + b51*b63 - b51*b75 + b52*b55 - b52*b64
     - b52*b76 + b53*b56 - b53*b65 - b53*b77 + b54*b57 - b54*b66 - b54*b78 + 
    b55*b58 - b55*b67 + b55*b79 + b56*b59 - b56*b68 + b56*b80 + b57*b60 - b57*
    b69 + b57*b81 - b58*b70 + b58*b82 - b59*b71 + b59*b83 - b60*b72 + b60*b84
     - b61*b64 + b61*b70 + b61*b85 - b62*b65 + b62*b71 + b62*b86 - b63*b66 + 
    b63*b72 + b63*b87 - b64*b67 - b64*b88 - b65*b68 - b65*b89 - b66*b69 - b66*
    b90 + b67*b70 - b67*b91 + b68*b71 - b68*b92 + b69*b72 - b69*b93 + b70*b94
     + b71*b95 + b72*b96 - b73*b76 - b73*b82 + b73*b85 - b74*b77 - b74*b83 + 
    b74*b86 - b75*b78 - b75*b84 + b75*b87 + b76*b79 + b76*b88 + b77*b80 + b77*
    b89 + b78*b81 + b78*b90 - b79*b82 - b79*b91 - b80*b83 - b80*b92 - b81*b84
     - b81*b93 - b82*b94 - b83*b95 - b84*b96 + b85*b88 - b85*b94 + b86*b89 - 
    b86*b95 + b87*b90 - b87*b96 + b88*b91 + b89*b92 + b90*b93 + b91*b94 + b92*
    b95 + b93*b96;

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

e28:    b82 + b83 + b84 = 1;

e29:    b85 + b86 + b87 = 1;

e30:    b88 + b89 + b90 = 1;

e31:    b91 + b92 + b93 = 1;

e32:    b94 + b95 + b96 = 1;

#  MINLP written by GAMS Convert at 01/12/18 13:34:27
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         37       37        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#        109        1      108        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        217      109      108        0
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
var b97 binary >= 0, <= 1;
var b98 binary >= 0, <= 1;
var b99 binary >= 0, <= 1;
var b100 binary >= 0, <= 1;
var b101 binary >= 0, <= 1;
var b102 binary >= 0, <= 1;
var b103 binary >= 0, <= 1;
var b104 binary >= 0, <= 1;
var b105 binary >= 0, <= 1;
var b106 binary >= 0, <= 1;
var b107 binary >= 0, <= 1;
var b108 binary >= 0, <= 1;

minimize obj: b1*b4 + b1*b16 + b1*b19 + b1*b91 + b2*b5 + b2*b17 + b2*b20 + b2*
    b92 + b3*b6 + b3*b18 + b3*b21 + b3*b93 + b4*b7 + b4*b22 + b4*b94 + b5*b8 + 
    b5*b23 + b5*b95 + b6*b9 + b6*b24 + b6*b96 + b7*b10 - b7*b25 + b7*b97 + b8*
    b11 - b8*b26 + b8*b98 + b9*b12 - b9*b27 + b9*b99 - b10*b13 + b10*b28 - b10*
    b100 - b11*b14 + b11*b29 - b11*b101 - b12*b15 + b12*b30 - b12*b102 + b13*
    b16 - b13*b31 + b13*b103 + b14*b17 - b14*b32 + b14*b104 + b15*b18 - b15*b33
     + b15*b105 + b16*b34 - b16*b106 + b17*b35 - b17*b107 + b18*b36 - b18*b108
     - b19*b22 + b19*b34 + b19*b37 - b20*b23 + b20*b35 + b20*b38 - b21*b24 + 
    b21*b36 + b21*b39 - b22*b25 - b22*b40 - b23*b26 - b23*b41 - b24*b27 - b24*
    b42 - b25*b28 + b25*b43 - b26*b29 + b26*b44 - b27*b30 + b27*b45 - b28*b31
     - b28*b46 - b29*b32 - b29*b47 - b30*b33 - b30*b48 - b31*b34 + b31*b49 - 
    b32*b35 + b32*b50 - b33*b36 + b33*b51 - b34*b52 - b35*b53 - b36*b54 + b37*
    b40 - b37*b52 - b37*b55 + b38*b41 - b38*b53 - b38*b56 + b39*b42 - b39*b54
     - b39*b57 + b40*b43 + b40*b58 + b41*b44 + b41*b59 + b42*b45 + b42*b60 - 
    b43*b46 - b43*b61 - b44*b47 - b44*b62 - b45*b48 - b45*b63 + b46*b49 - b46*
    b64 + b47*b50 - b47*b65 + b48*b51 - b48*b66 + b49*b52 - b49*b67 + b50*b53
     - b50*b68 + b51*b54 - b51*b69 - b52*b70 - b53*b71 - b54*b72 + b55*b58 + 
    b55*b70 - b55*b73 + b56*b59 + b56*b71 - b56*b74 + b57*b60 + b57*b72 - b57*
    b75 - b58*b61 - b58*b76 - b59*b62 - b59*b77 - b60*b63 - b60*b78 - b61*b64
     - b61*b79 - b62*b65 - b62*b80 - b63*b66 - b63*b81 - b64*b67 - b64*b82 - 
    b65*b68 - b65*b83 - b66*b69 - b66*b84 + b67*b70 + b67*b85 + b68*b71 + b68*
    b86 + b69*b72 + b69*b87 + b70*b88 + b71*b89 + b72*b90 + b73*b76 - b73*b88
     + b73*b91 + b74*b77 - b74*b89 + b74*b92 + b75*b78 - b75*b90 + b75*b93 + 
    b76*b79 + b76*b94 + b77*b80 + b77*b95 + b78*b81 + b78*b96 - b79*b82 - b79*
    b97 - b80*b83 - b80*b98 - b81*b84 - b81*b99 + b82*b85 - b82*b100 + b83*b86
     - b83*b101 + b84*b87 - b84*b102 - b85*b88 - b85*b103 - b86*b89 - b86*b104
     - b87*b90 - b87*b105 - b88*b106 - b89*b107 - b90*b108 + b91*b94 + b91*b106
     + b92*b95 + b92*b107 + b93*b96 + b93*b108 + b94*b97 + b95*b98 + b96*b99 - 
    b97*b100 - b98*b101 - b99*b102 - b100*b103 - b101*b104 - b102*b105 + b103*
    b106 + b104*b107 + b105*b108;

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

e33:    b97 + b98 + b99 = 1;

e34:    b100 + b101 + b102 = 1;

e35:    b103 + b104 + b105 = 1;

e36:    b106 + b107 + b108 = 1;

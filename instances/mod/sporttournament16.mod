#  MINLP written by GAMS Convert at 01/12/18 13:42:50
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        0        0        1        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#        121        1      120        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        121        1      120        0


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
var b109 binary >= 0, <= 1;
var b110 binary >= 0, <= 1;
var b111 binary >= 0, <= 1;
var b112 binary >= 0, <= 1;
var b113 binary >= 0, <= 1;
var b114 binary >= 0, <= 1;
var b115 binary >= 0, <= 1;
var b116 binary >= 0, <= 1;
var b117 binary >= 0, <= 1;
var b118 binary >= 0, <= 1;
var b119 binary >= 0, <= 1;
var b120 binary >= 0, <= 1;
var x121;

maximize obj: x121;

subject to

e1: 2*b1*b3 - 2*b1 - 2*b3 + 2*b1*b5 - 4*b5 + 2*b1*b38 + 2*b38 - 2*b1*b50 + 4*
    b50 + 2*b2*b5 - 2*b2 + 2*b2*b7 - 4*b7 + 2*b2*b25 - 2*b25 - 2*b2*b68 + 2*b68
     - 2*b3*b6 - 2*b6 + 2*b3*b26 - 2*b26 + 2*b3*b39 - 2*b39 + 2*b4*b7 - 4*b4 + 
    2*b4*b10 - 2*b10 + 2*b4*b18 - 2*b18 + 2*b4*b19 - 4*b19 + 2*b5*b8 - 4*b8 + 2
    *b5*b39 + 2*b6*b7 + 2*b6*b15 - 4*b15 + 2*b6*b99 + 2*b7*b11 - 2*b11 + 2*b8*
    b10 + 2*b8*b15 + 2*b8*b20 - 4*b20 + 2*b9*b23 - 2*b9 - 2*b23 + 2*b9*b64 - 4*
    b64 + 2*b10*b14 - 2*b14 - 2*b10*b100 + 2*b11*b20 + 2*b11*b27 - 4*b27 - 2*
    b11*b71 - 2*b71 + 2*b12*b20 - 2*b12 + 2*b12*b29 - 4*b29 - 2*b12*b99 + 2*b12
    *b104 + 2*b13*b18 - 2*b13 + 2*b13*b25 + 2*b14*b27 + 2*b14*b40 - 2*b40 - 2*
    b14*b51 - 2*b51 + 2*b15*b21 - 4*b21 + 2*b15*b75 - 2*b75 + 2*b16*b27 - 4*b16
     + 2*b16*b29 + 2*b16*b43 - 4*b43 + 2*b16*b99 + 2*b17*b48 - 2*b17 - 2*b48 + 
    2*b17*b102 + 2*b18*b26 - 2*b18*b98 + 2*b19*b26 + 2*b19*b103 + 2*b19*b106 + 
    2*b20*b28 - 2*b28 + 2*b21*b40 + 2*b21*b43 + 2*b21*b54 - 4*b54 - 2*b22*b23
     - 2*b22 + 2*b22*b47 - 4*b47 + 2*b22*b63 - 2*b63 + 2*b22*b107 + 2*b23*b24
     - 2*b24 + 2*b23*b62 - 4*b62 + 2*b24*b63 + 2*b25*b105 - 2*b25*b112 - 2*b26*
    b114 + 2*b27*b42 - 2*b42 + 2*b28*b54 - 2*b28*b74 - 2*b74 + 2*b28*b76 - 4*
    b76 + 2*b29*b30 - 2*b30 + 2*b29*b78 - 2*b78 + 2*b30*b31 - 4*b31 + 2*b30*b76
     - 2*b30*b80 + 2*b80 + 2*b31*b32 - 2*b32 + 2*b31*b78 + 2*b31*b90 + 2*b32*
    b61 - 2*b61 + 2*b32*b81 - 2*b81 - 2*b32*b107 + 2*b33*b35 - 2*b33 - 2*b35 + 
    2*b33*b60 - 2*b60 + 2*b33*b108 - 2*b33*b110 - 2*b34*b48 + 2*b34 + 2*b34*b87
     - 4*b87 - 2*b34*b89 - 2*b34*b94 + 2*b35*b61 + 2*b35*b87 - 2*b35*b102 + 2*
    b36*b37 - 2*b36 - 2*b37 + 2*b36*b86 - 4*b86 - 2*b36*b108 + 2*b36*b109 + 2*
    b37*b87 - 2*b38*b105 - 2*b38*b106 - 2*b38*b116 + 2*b39*b41 - 4*b41 - 2*b39*
    b99 + 2*b40*b53 - 4*b53 - 2*b40*b73 - 2*b73 + 2*b41*b52 - 2*b52 + 2*b41*b53
     + 2*b41*b75 + 2*b42*b44 - 2*b44 + 2*b42*b76 - 2*b42*b115 + 2*b43*b45 - 2*
    b45 + 2*b43*b56 + 2*b56 + 2*b44*b45 + 2*b44*b53 - 2*b44*b92 - 2*b45*b96 + 2
    *b45*b111 + 2*b46*b83 - 2*b46 - 2*b83 - 2*b46*b97 + 2*b46*b107 + 2*b46*b111
     + 2*b47*b82 - 2*b82 + 2*b47*b85 - 2*b85 + 2*b47*b110 + 2*b48*b108 + 2*b48*
    b118 - 2*b49*b68 + 2*b49 - 2*b49*b117 - 2*b50*b66 + 2*b66 - 2*b50*b69 - 2*
    b69 - 2*b50*b103 + 2*b51*b69 + 2*b51*b72 - 4*b72 + 2*b51*b116 + 2*b52*b70
     - 4*b70 - 2*b52*b114 + 2*b52*b115 + 2*b53*b55 - 2*b55 + 2*b54*b57 - 4*b57
     + 2*b54*b101 + 2*b55*b57 + 2*b55*b92 - 2*b55*b120 - 2*b56*b59 - 2*b59 - 2*
    b56*b97 - 2*b56*b104 + 2*b57*b59 + 2*b57*b96 - 2*b58*b60 + 2*b58 - 2*b58*
    b92 + 2*b58*b95 - 2*b58*b101 + 2*b59*b60 + 2*b59*b110 + 2*b60*b94 + 2*b61*
    b62 - 2*b61*b91 + 2*b62*b64 + 2*b62*b94 + 2*b63*b65 - 2*b65 - 2*b63*b93 + 2
    *b64*b65 + 2*b64*b93 - 2*b66*b113 + 2*b67*b68 - 2*b67 + 2*b67*b113 - 2*b68*
    b100 + 2*b69*b71 + 2*b69*b117 + 2*b70*b73 + 2*b70*b112 + 2*b70*b116 + 2*b71
    *b73 + 2*b71*b100 + 2*b72*b74 + 2*b72*b114 + 2*b72*b115 + 2*b73*b74 + 2*b74
    *b120 + 2*b75*b77 - 4*b77 - 2*b75*b104 + 2*b76*b79 - 4*b79 + 2*b77*b78 + 2*
    b77*b79 + 2*b77*b120 - 2*b78*b81 + 2*b79*b80 + 2*b79*b81 - 2*b80*b82 - 2*
    b80*b90 + 2*b81*b82 + 2*b82*b84 - 2*b84 + 2*b83*b85 + 2*b83*b86 - 2*b83*b95
     + 2*b84*b86 - 2*b84*b93 + 2*b84*b95 + 2*b85*b102 - 2*b85*b119 + 2*b86*b119
     + 2*b87*b88 - 2*b88 + 2*b88*b119 + 2*b89*b90 - 2*b89*b91 + 2*b89*b93 - 2*
    b90*b94 + 2*b91*b96 + 2*b91*b97 + 2*b92*b97 - 2*b95*b96 + 2*b98*b117 + 2*
    b100*b113 + 2*b101*b104 - 2*b101*b111 - 2*b102*b109 - 2*b107*b108 - 2*b110*
    b111 - 2*b112*b113 + 2*b112*b114 - 2*b115*b120 - 2*b116*b117 - 2*b118*b119
     + x121 <= 0;

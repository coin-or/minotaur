#  MINLP written by GAMS Convert at 01/12/18 13:30:48
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         23       13        3        7        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         77       14       63        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        174      166        8        0


var x1 >= 0;
var x2 >= 0;
var x3 >= 0;
var x4 := 1, >= 0.01, <= 20;
var x5 := 1, >= 0.01, <= 20;
var x6 := 1, >= 0.01, <= 20;
var x7 >= 0;
var x8 >= 0;
var x9 >= 0;
var x10 >= 0;
var x11 >= 0;
var x12 >= 0;
var x13 := 100, >= 0;
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
var x77;

minimize obj: x77;

subject to

e1: (-x13*x77) - ((416000 - 416000*exp(-0.1*x1/x4))*x4 + 37440*x1 - 100*x4 + (
    124615.384615385 - 124615.384615385*exp(-0.13*x2/x5))*x5 + 9000*x2 - 90*x5
     + (278666.666666667 - 278666.666666667*exp(-0.09*x3/x6))*x6 + 15840*x3 - 
    80*x6) = 0;

e2:  - 1300*x1 + x7 + 350*x13 = 0;

e3:  - 1000*x2 + x8 + 300*x13 = 0;

e4:  - 1100*x3 + x9 + 300*x13 = 0;

e5:    x7 - 300*x13 <= 0;

e6:    x8 - 300*x13 <= 0;

e7:    x9 - 300*x13 <= 0;

e8:    x4 - 0.01*b14 - b15 - 2*b16 - 3*b17 - 4*b18 - 5*b19 - 6*b20 - 7*b21
     - 8*b22 - 9*b23 - 10*b24 - 11*b25 - 12*b26 - 13*b27 - 14*b28 - 15*b29
     - 16*b30 - 17*b31 - 18*b32 - 19*b33 - 20*b34 = 0;

e9:    x5 - 0.01*b35 - b36 - 2*b37 - 3*b38 - 4*b39 - 5*b40 - 6*b41 - 7*b42
     - 8*b43 - 9*b44 - 10*b45 - 11*b46 - 12*b47 - 13*b48 - 14*b49 - 15*b50
     - 16*b51 - 17*b52 - 18*b53 - 19*b54 - 20*b55 = 0;

e10:    x6 - 0.01*b56 - b57 - 2*b58 - 3*b59 - 4*b60 - 5*b61 - 6*b62 - 7*b63
      - 8*b64 - 9*b65 - 10*b66 - 11*b67 - 12*b68 - 13*b69 - 14*b70 - 15*b71
      - 16*b72 - 17*b73 - 18*b74 - 19*b75 - 20*b76 = 0;

e11:  - b14 - b15 - b16 - b17 - b18 - b19 - b20 - b21 - b22 - b23 - b24 - b25
      - b26 - b27 - b28 - b29 - b30 - b31 - b32 - b33 - b34 = -1;

e12:  - b35 - b36 - b37 - b38 - b39 - b40 - b41 - b42 - b43 - b44 - b45 - b46
      - b47 - b48 - b49 - b50 - b51 - b52 - b53 - b54 - b55 = -1;

e13:  - b56 - b57 - b58 - b59 - b60 - b61 - b62 - b63 - b64 - b65 - b66 - b67
      - b68 - b69 - b70 - b71 - b72 - b73 - b74 - b75 - b76 = -1;

e14:  - x1 - 2*x4 + x10 = 0;

e15:  - x2 - 3*x5 + x11 = 0;

e16:  - x3 - 3*x6 + x12 = 0;

e17:    x10 + x11 + x12 - x13 <= 0;

e18:    x1 + 150*b14 <= 150;

e19:    x2 + 150*b35 <= 150;

e20:    x3 + 150*b56 <= 150;

e21:    x4 >= 1;

e22:    x5 >= 1;

e23:    x6 >= 1;

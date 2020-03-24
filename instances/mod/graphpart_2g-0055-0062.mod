#  MINLP written by GAMS Convert at 01/12/18 13:34:25
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

minimize obj: (-11669*b1*b4) - 144456*b1*b13 + 30835*b1*b16 - 33555*b1*b61 - 
    11669*b2*b5 - 144456*b2*b14 + 30835*b2*b17 - 33555*b2*b62 - 11669*b3*b6 - 
    144456*b3*b15 + 30835*b3*b18 - 33555*b3*b63 + 52284*b4*b7 + 145893*b4*b19
     - 140641*b4*b64 + 52284*b5*b8 + 145893*b5*b20 - 140641*b5*b65 + 52284*b6*
    b9 + 145893*b6*b21 - 140641*b6*b66 + 127397*b7*b10 - 12640*b7*b22 + 217400*
    b7*b67 + 127397*b8*b11 - 12640*b8*b23 + 217400*b8*b68 + 127397*b9*b12 - 
    12640*b9*b24 + 217400*b9*b69 + 50562*b10*b13 - 78959*b10*b25 - 36333*b10*
    b70 + 50562*b11*b14 - 78959*b11*b26 - 36333*b11*b71 + 50562*b12*b15 - 78959
    *b12*b27 - 36333*b12*b72 + 98784*b13*b28 - 31218*b13*b73 + 98784*b14*b29 - 
    31218*b14*b74 + 98784*b15*b30 - 31218*b15*b75 + 6320*b16*b19 + 171989*b16*
    b28 - 39163*b16*b31 + 6320*b17*b20 + 171989*b17*b29 - 39163*b17*b32 + 6320*
    b18*b21 + 171989*b18*b30 - 39163*b18*b33 - 183413*b19*b22 + 60462*b19*b34
     - 183413*b20*b23 + 60462*b20*b35 - 183413*b21*b24 + 60462*b21*b36 + 45778*
    b22*b25 + 192287*b22*b37 + 45778*b23*b26 + 192287*b23*b38 + 45778*b24*b27
     + 192287*b24*b39 + 125824*b25*b28 + 41371*b25*b40 + 125824*b26*b29 + 41371
    *b26*b41 + 125824*b27*b30 + 41371*b27*b42 + 17724*b28*b43 + 17724*b29*b44
     + 17724*b30*b45 + 12200*b31*b34 + 106251*b31*b43 - 88012*b31*b46 + 12200*
    b32*b35 + 106251*b32*b44 - 88012*b32*b47 + 12200*b33*b36 + 106251*b33*b45
     - 88012*b33*b48 - 17583*b34*b37 + 118167*b34*b49 - 17583*b35*b38 + 118167*
    b35*b50 - 17583*b36*b39 + 118167*b36*b51 - 5402*b37*b40 - 27062*b37*b52 - 
    5402*b38*b41 - 27062*b38*b53 - 5402*b39*b42 - 27062*b39*b54 - 117425*b40*
    b43 - 125320*b40*b55 - 117425*b41*b44 - 125320*b41*b56 - 117425*b42*b45 - 
    125320*b42*b57 + 29332*b43*b58 + 29332*b44*b59 + 29332*b45*b60 + 69937*b46*
    b49 + 65415*b46*b58 - 197304*b46*b61 + 69937*b47*b50 + 65415*b47*b59 - 
    197304*b47*b62 + 69937*b48*b51 + 65415*b48*b60 - 197304*b48*b63 + 66551*b49
    *b52 + 92080*b49*b64 + 66551*b50*b53 + 92080*b50*b65 + 66551*b51*b54 + 
    92080*b51*b66 + 47821*b52*b55 + 68105*b52*b67 + 47821*b53*b56 + 68105*b53*
    b68 + 47821*b54*b57 + 68105*b54*b69 - 116706*b55*b58 - 6426*b55*b70 - 
    116706*b56*b59 - 6426*b56*b71 - 116706*b57*b60 - 6426*b57*b72 - 154797*b58*
    b73 - 154797*b59*b74 - 154797*b60*b75 - 14848*b61*b64 - 2103*b61*b73 - 
    14848*b62*b65 - 2103*b62*b74 - 14848*b63*b66 - 2103*b63*b75 + 17019*b64*b67
     + 17019*b65*b68 + 17019*b66*b69 + 83365*b67*b70 + 83365*b68*b71 + 83365*
    b69*b72 - 20293*b70*b73 - 20293*b71*b74 - 20293*b72*b75;

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

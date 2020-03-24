#  MINLP written by GAMS Convert at 01/12/18 13:34:25
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         17       17        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         49        1       48        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         97       49       48        0
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

minimize obj: 13960*b1*b4 + 132180*b1*b10 - 45455*b1*b13 + 21397*b1*b37 + 13960
    *b2*b5 + 132180*b2*b11 - 45455*b2*b14 + 21397*b2*b38 + 13960*b3*b6 + 132180
    *b3*b12 - 45455*b3*b15 + 21397*b3*b39 - 30411*b4*b7 - 101527*b4*b16 - 8896*
    b4*b40 - 30411*b5*b8 - 101527*b5*b17 - 8896*b5*b41 - 30411*b6*b9 - 101527*
    b6*b18 - 8896*b6*b42 - 122332*b7*b10 + 30058*b7*b19 + 33114*b7*b43 - 122332
    *b8*b11 + 30058*b8*b20 + 33114*b8*b44 - 122332*b9*b12 + 30058*b9*b21 + 
    33114*b9*b45 - 13676*b10*b22 + 124553*b10*b46 - 13676*b11*b23 + 124553*b11*
    b47 - 13676*b12*b24 + 124553*b12*b48 + 17153*b13*b16 + 73691*b13*b22 - 
    32022*b13*b25 + 17153*b14*b17 + 73691*b14*b23 - 32022*b14*b26 + 17153*b15*
    b18 + 73691*b15*b24 - 32022*b15*b27 - 106129*b16*b19 - 101395*b16*b28 - 
    106129*b17*b20 - 101395*b17*b29 - 106129*b18*b21 - 101395*b18*b30 - 10958*
    b19*b22 - 41751*b19*b31 - 10958*b20*b23 - 41751*b20*b32 - 10958*b21*b24 - 
    41751*b21*b33 - 30446*b22*b34 - 30446*b23*b35 - 30446*b24*b36 + 94519*b25*
    b28 + 44613*b25*b34 + 70141*b25*b37 + 94519*b26*b29 + 44613*b26*b35 + 70141
    *b26*b38 + 94519*b27*b30 + 44613*b27*b36 + 70141*b27*b39 + 73611*b28*b31 - 
    54792*b28*b40 + 73611*b29*b32 - 54792*b29*b41 + 73611*b30*b33 - 54792*b30*
    b42 + 57663*b31*b34 - 147596*b31*b43 + 57663*b32*b35 - 147596*b32*b44 + 
    57663*b33*b36 - 147596*b33*b45 + 70863*b34*b46 + 70863*b35*b47 + 70863*b36*
    b48 + 24474*b37*b40 - 178500*b37*b46 + 24474*b38*b41 - 178500*b38*b47 + 
    24474*b39*b42 - 178500*b39*b48 + 118713*b40*b43 + 118713*b41*b44 + 118713*
    b42*b45 + 34279*b43*b46 + 34279*b44*b47 + 34279*b45*b48;

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

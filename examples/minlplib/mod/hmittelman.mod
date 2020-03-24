#  MINLP written by GAMS Convert at 01/12/18 13:34:33
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          8        1        0        7        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         17        1       16        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#        123        1      122        0
# 
#  Reformulation has removed 1 variable and 1 equation


var b1 binary := 0.171747132, >= 0, <= 1;
var b2 binary := 0.843266708, >= 0, <= 1;
var b3 binary := 0.550375356, >= 0, <= 1;
var b4 binary := 0.301137904, >= 0, <= 1;
var b5 binary := 0.292212117, >= 0, <= 1;
var b6 binary := 0.224052867, >= 0, <= 1;
var b7 binary := 0.349830504, >= 0, <= 1;
var b8 binary := 0.998117627, >= 0, <= 1;
var b9 binary := 0.578733378, >= 0, <= 1;
var b10 binary := 0.991133039, >= 0, <= 1;
var b11 binary := 0.130692483, >= 0, <= 1;
var b12 binary := 0.639718759, >= 0, <= 1;
var b13 binary := 0.159517864, >= 0, <= 1;
var b14 binary := 0.250080533, >= 0, <= 1;
var b15 binary := 0.668928609, >= 0, <= 1;
var b16 binary := 0.435356381, >= 0, <= 1;

minimize obj: 10*b5*b7*b9*b10*b14*b15*b16 + 7*b1*b2*b3*b4*b8*b11 + b3*b4*b6*b7*
    b8 + 12*b3*b4*b8*b11 + 8*b6*b7*b8*b12 + 3*b6*b7*b9*b14*b16 + b9*b10*b14*b16
     + 5*b5*b10*b14*b15*b16 + 3*b1*b2*b11*b12;

subject to

e2: 3*b5*b7*b9*b10*b14*b15*b16 - 12*b1*b2*b3*b4*b8*b11 - 8*b3*b4*b6*b7*b8 + b3*
    b4*b8*b11 - 7*b1*b2*b11*b12 + 2*b13*b14*b15*b16 <= -2;

e3: b1*b2*b3*b4*b8*b11 - 10*b3*b4*b6*b7*b8 - 5*b6*b7*b8*b12 + b6*b7*b9*b14*b16
     + 7*b9*b10*b14*b16 + b5*b10*b14*b15*b16 <= -1;

e4: 5*b5*b7*b9*b10*b14*b15*b16 - 3*b1*b2*b3*b4*b8*b11 - b3*b4*b6*b7*b8 - 2*b5*
    b10*b14*b15*b16 + b13*b14*b15*b16 <= -1;

e5: 3*b1*b2*b3*b4*b8*b11 - 5*b5*b7*b9*b10*b14*b15*b16 + b3*b4*b6*b7*b8 + 2*b5*
    b10*b14*b15*b16 - b13*b14*b15*b16 <= 1;

e6: (-4*b3*b4*b6*b7*b8) - 2*b3*b4*b8*b11 - 5*b6*b7*b9*b14*b16 + b9*b10*b14*b16
     - 9*b5*b10*b14*b15*b16 - 2*b1*b2*b11*b12 <= -3;

e7: 9*b1*b2*b3*b4*b8*b11 - 12*b3*b4*b8*b11 - 7*b6*b7*b8*b12 + 6*b6*b7*b9*b14*
    b16 + 2*b5*b10*b14*b15*b16 - 15*b1*b2*b11*b12 + 3*b13*b14*b15*b16 <= -7;

e8: 5*b1*b2*b3*b4*b8*b11 - 8*b5*b7*b9*b10*b14*b15*b16 + 2*b3*b4*b6*b7*b8 - 7*b3
    *b4*b8*b11 - b6*b7*b8*b12 - 5*b9*b10*b14*b16 - 10*b1*b2*b11*b12 <= -1;

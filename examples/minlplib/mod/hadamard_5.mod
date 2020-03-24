#  MINLP written by GAMS Convert at 12/08/18 18:03:34
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        0        1        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         26        1       25        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         26        1       25        0


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
var x26;

maximize obj: x26;

subject to

e1: b1*b7*b13*b19*b25 - b1*b7*b13*b20*b24 + b1*b7*b15*b18*b24 - b1*b10*b12*b18*
    b24 + b5*b6*b12*b18*b24 - b5*b6*b12*b19*b23 + b1*b10*b12*b19*b23 - b1*b7*
    b15*b19*b23 + b1*b7*b14*b20*b23 - b1*b7*b14*b18*b25 + b1*b9*b12*b18*b25 - 
    b1*b9*b12*b20*b23 + b1*b9*b15*b17*b23 - b1*b10*b14*b17*b23 + b5*b6*b14*b17*
    b23 - b5*b9*b11*b17*b23 + b4*b10*b11*b17*b23 - b4*b6*b15*b17*b23 + b4*b6*
    b12*b20*b23 - b4*b6*b12*b18*b25 + b4*b6*b13*b17*b25 - b4*b6*b13*b20*b22 + 
    b4*b6*b15*b18*b22 - b4*b10*b11*b18*b22 + b5*b9*b11*b18*b22 - b5*b6*b14*b18*
    b22 + b1*b10*b14*b18*b22 - b1*b9*b15*b18*b22 + b1*b9*b13*b20*b22 - b1*b9*
    b13*b17*b25 + b1*b8*b14*b17*b25 - b1*b8*b14*b20*b22 + b1*b8*b15*b19*b22 - 
    b1*b10*b13*b19*b22 + b5*b6*b13*b19*b22 - b5*b6*b13*b17*b24 + b1*b10*b13*b17
    *b24 - b1*b8*b15*b17*b24 + b1*b8*b12*b20*b24 - b1*b8*b12*b19*b25 + b3*b6*
    b12*b19*b25 - b3*b6*b12*b20*b24 + b3*b6*b15*b17*b24 - b3*b10*b11*b17*b24 + 
    b5*b8*b11*b17*b24 - b5*b8*b11*b19*b22 + b3*b10*b11*b19*b22 - b3*b6*b15*b19*
    b22 + b3*b6*b14*b20*b22 - b3*b6*b14*b17*b25 + b3*b9*b11*b17*b25 - b3*b9*b11
    *b20*b22 + b3*b9*b15*b16*b22 - b3*b10*b14*b16*b22 + b5*b8*b14*b16*b22 - b5*
    b9*b13*b16*b22 + b4*b10*b13*b16*b22 - b4*b8*b15*b16*b22 + b4*b8*b11*b20*b22
     - b4*b8*b11*b17*b25 + b4*b8*b12*b16*b25 - b4*b8*b12*b20*b21 + b4*b8*b15*
    b17*b21 - b4*b10*b13*b17*b21 + b5*b9*b13*b17*b21 - b5*b8*b14*b17*b21 + b3*
    b10*b14*b17*b21 - b3*b9*b15*b17*b21 + b3*b9*b12*b20*b21 - b3*b9*b12*b16*b25
     + b3*b7*b14*b16*b25 - b3*b7*b14*b20*b21 + b3*b7*b15*b19*b21 - b3*b10*b12*
    b19*b21 + b5*b8*b12*b19*b21 - b5*b8*b12*b16*b24 + b3*b10*b12*b16*b24 - b3*
    b7*b15*b16*b24 + b3*b7*b11*b20*b24 - b3*b7*b11*b19*b25 + b2*b8*b11*b19*b25
     - b2*b8*b11*b20*b24 + b2*b8*b15*b16*b24 - b2*b10*b13*b16*b24 + b5*b7*b13*
    b16*b24 - b5*b7*b13*b19*b21 + b2*b10*b13*b19*b21 - b2*b8*b15*b19*b21 + b2*
    b8*b14*b20*b21 - b2*b8*b14*b16*b25 + b2*b9*b13*b16*b25 - b2*b9*b13*b20*b21
     + b2*b9*b15*b18*b21 - b2*b10*b14*b18*b21 + b5*b7*b14*b18*b21 - b5*b9*b12*
    b18*b21 + b4*b10*b12*b18*b21 - b4*b7*b15*b18*b21 + b4*b7*b13*b20*b21 - b4*
    b7*b13*b16*b25 + b4*b7*b11*b18*b25 - b4*b7*b11*b20*b23 + b4*b7*b15*b16*b23
     - b4*b10*b12*b16*b23 + b5*b9*b12*b16*b23 - b5*b7*b14*b16*b23 + b2*b10*b14*
    b16*b23 - b2*b9*b15*b16*b23 + b2*b9*b11*b20*b23 - b2*b9*b11*b18*b25 + b2*b6
    *b14*b18*b25 - b2*b6*b14*b20*b23 + b2*b6*b15*b19*b23 - b2*b10*b11*b19*b23
     + b5*b7*b11*b19*b23 - b5*b7*b11*b18*b24 + b2*b10*b11*b18*b24 - b2*b6*b15*
    b18*b24 + b2*b6*b13*b20*b24 - b2*b6*b13*b19*b25 - x26 >= 0;

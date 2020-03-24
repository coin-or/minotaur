#  MINLP written by GAMS Convert at 01/12/18 13:46:59
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#         11        6        0        5        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#         21       11       10        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#         51       21       30        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 68, >= 68, <= 71;
var x2 := 65, >= 65, <= 87;
var x3 := 107, >= 107, <= 126;
var x4 := 38, >= 38, <= 49;
var x5 := 40, >= 40, <= 55;
var x6 := 54, >= 54, <= 68;
var x7 := 92, >= 92, <= 106;
var x8 := 113, >= 113, <= 117;
var x9 := 82, >= 82, <= 87;
var x10 := 76, >= 76, <= 85;
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

minimize obj: sqrt((x1 - x3)^2 + (x2 - x4)^2)*b11 + sqrt((x1 - x5)^2 + (x2 - x6
    )^2)*b12 + sqrt((x1 - x7)^2 + (x2 - x8)^2)*b13 + sqrt((x1 - x9)^2 + (x2 - 
    x10)^2)*b14 + sqrt((x3 - x5)^2 + (x4 - x6)^2)*b15 + sqrt((x3 - x7)^2 + (x4
     - x8)^2)*b16 + sqrt((x3 - x9)^2 + (x4 - x10)^2)*b17 + sqrt((x5 - x7)^2 + (
    x6 - x8)^2)*b18 + sqrt((x5 - x9)^2 + (x6 - x10)^2)*b19 + sqrt((x7 - x9)^2
     + (x8 - x10)^2)*b20;

subject to

e2: 0.444444444444444*x1^2 - 61.7777777777778*x1 + 0.00826446280991736*x2^2 - 
    1.25619834710744*x2 <= -2193.51331496786;

e3: 0.0110803324099723*x3^2 - 2.58171745152355*x3 + 0.0330578512396694*x4^2 - 
    2.87603305785124*x4 <= -211.938760559511;

e4: 0.0177777777777778*x5^2 - 1.68888888888889*x5 + 0.0204081632653061*x6^2 - 
    2.48979591836735*x6 <= -115.049886621315;

e5: 0.0204081632653061*x7^2 - 4.04081632653061*x7 + 0.25*x8^2 - 57.5*x8
     <= -3505.27040816327;

e6: 0.16*x9^2 - 27.04*x9 + 0.0493827160493827*x10^2 - 7.95061728395062*x10
     <= -1461.45234567901;

e7:    b11 + b12 + b13 + b14 = 2;

e8:    b11 + b15 + b16 + b17 = 2;

e9:    b12 + b15 + b18 + b19 = 2;

e10:    b13 + b16 + b18 + b20 = 2;

e11:    b14 + b17 + b19 + b20 = 2;

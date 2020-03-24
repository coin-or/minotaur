#  NLP written by GAMS Convert at 01/12/18 13:33:44
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          2        2        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          3        3        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          5        2        3        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 := 900, >= 900;
var x2 := 10, >= 10;

minimize obj: 116*(10000000/((10 + x1/x2)*(-288000 + 1440*x1)))**0.86 + 47300*
    x1/(-200 + x1) - 47300;

subject to

e2: -2100*log10(41.1522633744856/x2) + x1 = 0;

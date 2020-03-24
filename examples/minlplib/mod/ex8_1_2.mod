#  NLP written by GAMS Convert at 01/12/18 13:32:00
#  
#  Equation counts
#      Total        E        G        L        N        X        C        B
#          1        1        0        0        0        0        0        0
#  
#  Variable counts
#                   x        b        i      s1s      s2s       sc       si
#      Total     cont   binary  integer     sos1     sos2    scont     sint
#          2        2        0        0        0        0        0        0
#  FX      0        0        0        0        0        0        0        0
#  
#  Nonzero counts
#      Total    const       NL      DLL
#          2        1        1        0
# 
#  Reformulation has removed 1 variable and 1 equation


var x1 >= 0, <= 6.28318;

minimize obj: 588600/(10.8095222429746 - 4.21478541710781*cos((-
    2.09439333333333) + x1))^6 - 1079.1/(10.8095222429746 - 4.21478541710781*
    cos((-2.09439333333333) + x1))^3 + 600800/(10.8095222429746 - 
    4.21478541710781*cos(x1))^6 - 1071.5/(10.8095222429746 - 4.21478541710781*
    cos(x1))^3 + 481300/(10.8095222429746 - 4.21478541710781*cos(
    2.09439333333333 + x1))^6 - 1064.6/(10.8095222429746 - 4.21478541710781*
    cos(2.09439333333333 + x1))^3;

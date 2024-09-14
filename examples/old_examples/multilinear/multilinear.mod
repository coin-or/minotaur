
Variables z,x2,x3,x4,x5,x6,x7,x8,x9,x10,x11,x12,x13,x14,x15; 
Positive Variables x2,x3,x4,x5,x6; 
Equations e1,e2,e3,e4,e5,e6,e7,e8; 

e1.. 6.3*x5*x8 + z - 5.04*x2 - 0.35*x3 - x4 - 3.36*x6 =E= 0; 

e2.. - 0.819672131147541*x2 + x5 - 0.819672131147541*x6 =E= 0; 

e3.. 0.98*x4 - x7*(0.01*x5*x10 + x4) =E= 0; 

e4.. - x2*x2*x3*x9 + 10*x2*x3*x3 + x2*x3*x6 =E= 0; 

e5.. x5*x12 - x2*(1.12 + 0.13167*x9 - 0.0067*x9*x9) =E= 0; 

e6.. x8*x8*x13 - 0.01*x8*(1.098*x9 - 0.038*x9*x9) - 0.325*x7*x8 =G= 0.516825; 

e7.. x2*x10*x14 + 22.2*x2*x11 =E= 71.64; 

e8.. -x6*x11*x15 + 3*x6*x8 =L= 2.66; 

* set non default bounds 

x2.up = 2; 
x3.up = 1.6; 
x4.up = 1.2; 
x5.up = 5; 
x6.up = 2; 
x7.lo = 0.85; 
x7.up = 0.93; 
x8.lo = 0.9; 
x8.up = 0.95; 
x9.lo = 3; 
x9.up = 12; 
x10.lo = 1.2; 
x10.up = 4; 
x11.lo = 1.45; 
x11.up = 1.62; 
x12.lo = 0.99; 
x12.up = 1.01010101010101; 
x13.lo = 0.99; 
x13.up = 1.01010101010101; 
x14.lo = 0.9; 
x14.up = 1.11111111111111; 
x15.lo = 0.99; 
x15.up = 1.01010101010101; 


* set non default marginals 

Model m / all /; 
m.limrow=0; 
m.limcol=0; 
$if NOT '%gams.u1%' == '' $include '%gams.u1%' 
Solve m using NLP minimizing z; 
var x1 >=0, <= 1;
var x2 >=0, <= 1;
var x3 >=0, <= 1;
var x4 >=0, <= 1;

minimize obj: x1 + 2*x2 + 3*x3 + 4*x4;

subject to

e1: 0.98*x1 - (0.01*x2*x3 + x1)*x4 = 0;

e2: x1*x2*x2*x3*x4 >= 0.01;

e3: x2^2*x3 + 0.2*x1*x3^3+ x4 <=3;
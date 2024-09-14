var x1 >=0, <= 1;
var x2 >=0, <= 1;
var x3 >=0, <= 1;
var x4 >=0, <= 1;

minimize obj: x1*x2 + 2*x2*x3 + 3*x1*x3;

subject to

e2: x1*x2*x3 + 10*x2*x3*x4 >=0.01;

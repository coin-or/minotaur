var x1 binary;
var x2 binary;
var x3 >=0, <= 1;

minimize obj: x1 + 2*x2;

subject to

e2: 2*x1*x1*x1 + 7*x2*x2*x3  >= 2;

e3: 5*x1*x1 + 3*x2  <= 3;

e4: x1 + x1^2*x2 + x1*x2 + x1^3 >= 0.01;


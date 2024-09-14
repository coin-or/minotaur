var x1 >=0, <= 1;
var x2 >=0, <= 1;
var x3 >=0, <= 1;

minimize obj: x1 + 2*x2;

subject to

e2: 2*x1 + 7*x2 + x3^3  >= 2;

e3: 5*x1 + 3*x2  <= 3;

# This is our first simple little quadratic MINLP

var v;
var x{1..3} >= 0, <= 1;


minimize obj: 2*x[1] - 3*x[2] + v;

subject to c1:
	v >= 4*x[1]*x[2] - 4*x[1]*x[3] + x[1]*x[1]
	  + 18*x[2]*x[3]	  ;









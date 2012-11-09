# This is our first simple little quadratic MINLP


var x{1..3} >= 0, <= 1;
var z{1..3} binary;
var v;

minimize obj: v ;

subject to c1:
	v >= 4*x[1]*x[1] + 2*x[1]*x[2] - 2*x[1]*x[3] + 2*x[2]*x[1]
	  + 17*x[2]*x[2] - 9*x[2]*x[3] - 2*x[1]*x[3] - 9*x[2]*x[3]
	  + 14*x[3]*x[3] ;

subject to vub{j in 1..3}:
        x[j] <= z[j] ;

subject to budget:
	sum{j in 1..3} x[j] = 1 ;

subject to card:
	sum{j in 1..3} z[2] <= 2 ;








var x{i in 1..1000} >=0, <=1;

minimize cost: sum{i in 1..1000} x[i];
subject to cons1: sum{i in 1..1000} x[i]*x[i] >= 2;

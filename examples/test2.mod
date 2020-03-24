var x{i in 1..100000} >=0, <=1;

minimize cost: sum{i in 1..100000} x[i];
subject to cons1: sum{i in 1..50000} (x[i]*x[i+50000]) + x[50000]*x[50001] >= 2;

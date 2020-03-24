var x{i in 1..1000} >=0, <=1;

minimize cost: sum{i in 1..1000} x[i];
subject to cons1: sum{i in 1..250} (x[i]*x[i+250] + x[i+250]*x[i+500] + x[i+500]*x[i+750] + x[i]*x[i+750])
                  + sum{j in 1..249} x[j]*x[j+1]  >= 2;

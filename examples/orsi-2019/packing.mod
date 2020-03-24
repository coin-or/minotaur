param NUM_CIRCLES;
param RADII{k in 1..NUM_CIRCLES};
param TRIANGLE_SIDE;

var x{k in 1..NUM_CIRCLES};
var y{k in 1..NUM_CIRCLES};

minimize cost: 0;
s.t. cons1{k in 1..NUM_CIRCLES}: -x[k]<=-RADII[k];
s.t. cons2{k in 1..NUM_CIRCLES}: -y[k]<=-RADII[k];
s.t. cons3{k in 1..NUM_CIRCLES}: x[k]+y[k]<=TRIANGLE_SIDE-sqrt(2)*RADII[k];
s.t. cons4{i in 1..NUM_CIRCLES, j in 1..NUM_CIRCLES: j>i}: (x[i]-x[j])^2 + (y[i]-y[j])^2>=(RADII[i]+RADII[j])^2

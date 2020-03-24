param NUM_CIRCLES;
param RADII{k in 1..NUM_CIRCLES};
#param side_up;
param TRIANGLE_SIDE;
param M;

var x{k in 1..NUM_CIRCLES};
var y{k in 1..NUM_CIRCLES};
var z{k in 1..NUM_CIRCLES} binary;

maximize num: sum{k in 1..NUM_CIRCLES} z[k];
s.t. cons1{k in 1..NUM_CIRCLES}: -x[k]<=-RADII[k];
s.t. cons2{k in 1..NUM_CIRCLES}: -y[k]<=-RADII[k];
s.t. cons3{k in 1..NUM_CIRCLES}: x[k]+y[k]<=TRIANGLE_SIDE-sqrt(2)*RADII[k];
s.t. cons4{i in 1..NUM_CIRCLES, j in 1..NUM_CIRCLES: j>i}: (x[i]-x[j])^2 + (y[i]-y[j])^2
                                                           + ((RADII[i]+RADII[j])^2)*(2-z[i]-z[j])
                                                           >=(RADII[i]+RADII[j])^2;
#s.t. cons5: z[1]==1;

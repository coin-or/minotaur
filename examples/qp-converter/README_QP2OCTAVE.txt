qp2octave converts a continuous QP from .nl format to octave readable file.

Usage: ./qp2octave xyz.nl

It creates a file qp.m. If the file already exists, it overwrites it. Infinity
is denoted by (+/-) 1e20.

A sample output file is:

%Variables
n = 3

%Constraints
m = 1

% variable lower bounds 
vlb = [0.000000 0.000000 0.000000 ];

% variable upper bounds 
vub = [50.000000 50.000000 50.000000 ];

% constraint lower bounds 
clb = [2.000000 ];

% constraint upper bounds 
cub = [100000000000000000000.000000 ];

% cost vector 
cv = [0.000000 0.000000 0.000000 ];

% constraint matrix 
Arows = [0 0 0 ];
Acols = [0 1 2 ];
Avals = [2.000000 1.000000 0.200000 ];

% Hessian matrix 
Hrows = [0 1 2 ];
Hcols = [0 1 2 ];
Hvals = [2.000000 2.000000 2.000000 ];


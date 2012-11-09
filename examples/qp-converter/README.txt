%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% qp-conveter takes a qcqp or a qp through a .nl format file
% and extracts the information needed to build the sdp 
% relaxation of the problem. This information is written in
% a file with the format that follows. Right now we have a
% Matlab code that reads this file and builds and solves
% the sdp relaxation of the problem.
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%number of variables
4
%number of Quadratic constraints
5
%number of linear constraints
2
% Q of the objective (matrix of the quadratic part of the objective)
1 2 3 4
1 3 2 5
6 5 4 1
9 7 6 1
% L of the objective (vector of the coefficients of the linear part of the objective)
2 3 6 5

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% sign, bound, Q, and L of quadratic constraint
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% REPEAT as many times as the number of quadratic constraints

% sign of the constraint (1: equality -- 2: less than or equal -- 3: greater than or equal)
1

% bound of the constraint (rhs of the constraint)
-4

% Q of the quadratic constraint (matrix of the quadratic part of the constraint)
1 2 3 4
1 3 2 5
6 5 4 1
9 7 6 1

% L of the constraint (vector of the coefficients of the linear part of the constraint)
2 3 6 5

% END REPEAT
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% sign, bound, and L of linear constraint
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% REPEAT as many times as the number of linear constraints

% sign of the constraint (1: equality -- 2: less than or equal -- 3: greater than or equal)
1

% bound of the constraint (rhs of the constraint)
-4

% L of the constraint (vector of the coefficients of the linear part of the constraint)
2 3 6 5

% END REPEAT
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% lower bound on sdp variable: [1 x'; x xx']

1 0 0 0 0
0 0 0 0 0 
0 0 0 0 0 
0 0 0 0 0
0 0 0 0 0

% upper bound on sdp variable: [1 x'; x xx']

1 1 1 1 1
1 1 1 1 1
1 1 1 1 1
1 1 1 1 1
1 1 1 1 1

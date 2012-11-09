Multilinear test format:

# Number vars, 
# lb, ub for each var
# num terms in (linear) objective.  1 objective only
#  ix, val, for objective
# # linear constraints
# for each linear constrint...
#    num terms in (linear) constraint, 
#    terms
#    followed by 'L/G/E' RHS
# # multilinear constraints
# for each multilinear constraint...
#   Num multilinear terms, num quadratic terms, num linear terms
#     coef, # vars in multilinear terms, ix of vars in multilinear term
#     quadratic terms
#     linear terms
#     followed by 'L/G/E' RHS


# Hacky stuff for trying to get numbers for ISMP
lower and upper bounds 666, mean it is an "eta"/"z" variable, not to be fixed.
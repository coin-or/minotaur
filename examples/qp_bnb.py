# //
# //     MINOTAUR -- It's only 1/2 bull
# //
# //     (C)opyright 2008--2010 The MINOTAUR Team
# //

# // Contributer: Sumanto Kar(jeetsumanto123@gmail.com)

# Quadratic Program Example
# ==============================================
# var x1 >= -10 <= 10  integer ;
# var x2 >= -10 <= 10 ;
# minimize obj: + 1*x1*x1 - 1*x2*x2  - 0;

# subject to cons0: 1 <= + 1*x1 + 1*x2 ;
# ===============================================
solver_path = '../build/bin/mbnb' #The solver path to be added by the user
from pyomo.environ import *
from pyomo.opt import SolverFactory

# Create a Pyomo model
model = ConcreteModel()

# Define decision variables
model.x1 = Var(within=Integers, initialize=0.0, bounds=(-10, 10))  # Integer variable
model.x2 = Var(within=Reals, initialize=1.0, bounds=(-10, 10))   # Continuous variable

# Objective: Minimize x1^2 + x2^2 (which simplifies to x1 + x2 for binary and bounded variables)
model.obj = Objective(expr=model.x1**2 + model.x2**2, sense=maximize)

# Constraint: x1 + x2 >= 1
model.con = Constraint(expr=model.x1 + model.x2 >= 1)

# Solve the problem using  BnB
solver = SolverFactory("mbnb", executable=solver_path) 

# Solve the problem
solver.solve(model, tee=True)

# Display the results
model.display()

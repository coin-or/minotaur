# //
# //     MINOTAUR -- It's only 1/2 bull
# //
# //     (C)opyright 2008--2010 The MINOTAUR Team
# //

# // Contributer: Sumanto Kar(jeetsumanto123@gmail.com)

# ################################################
# Mixed Integer Non-Linear Program
# ################################################
# var x1 >= 0 <= 1  binary ;
# var x2 >= -10 <= 10 ;
# minimize obj: + 1*x1*x1 + 1*x2*x2  - 0;

# subject to cons0:  +  nonlinear function  <= 1;

# subject to cons1: 1 <= + 1*x1 + 1*x2 ;
# #################################################
# Where nonlinear function = x1 * x2^2
# #################################################
solver_path = '../build/bin/mqg' #The solver path to be added by the user
from pyomo.environ import *
from pyomo.opt import SolverFactory

# Create a Pyomo model
model = ConcreteModel()

# Define decision variables
model.x1 = Var(within=Binary, initialize=0)    # Binary variable x1 (0 or 1)
model.x2 = Var(within=Reals, bounds=(-10, 10), initialize=0.0)  # Continuous variable x2

# Define the objective function: Minimize x1^2 + x2^2
model.obj = Objective(expr=model.x1**2 + model.x2**2, sense=minimize)

# Nonlinear constraint: x1 * x2^2 <= 1
model.nl_con = Constraint(expr=model.x1 * model.x2**2 <= 1)

# Linear constraint: x1 + x2 >= 1
model.lin_con = Constraint(expr=model.x1 + model.x2 >= 1)

# Solve the problem
solver = SolverFactory("mqg", executable=solver_path) 
results = solver.solve(model, tee=True)

# Display the results
model.display()

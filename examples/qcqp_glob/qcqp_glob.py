
# =======================================================================
# Quadratically Constrained Quadratic Program (QCQP) 
# var x1 >= -10 <= 10 ;
# var x2 >= -10 <= 10 ;
# maximize obj: + 0.5*x1*x1 + 0.5*x2*x2  - 0;

# subject to cons0: + 0.5*x1*x1 + 1*x1*x2 - 1*x2*x2  <= 1;
# =========================================================================

from pyomo.environ import *

# Create a model
model = ConcreteModel()

# Define variables
model.x1 = Var(within=Reals, bounds=(-10, 10), initialize=0.0)
model.x2 = Var(within=Reals, bounds=(-10, 10), initialize=0.0)

# Define the objective function
model.obj = Objective(expr=0.5 * model.x1**2 + 0.5 * model.x2**2, sense=maximize)

# Define the constraint
model.cons = Constraint(expr=0.5 * model.x1**2 + model.x1 * model.x2 - model.x2**2 <= 1)

# Create a solver
solver = SolverFactory("mglob", executable='../../build/bin/mglob') 

# Solve the problem
results = solver.solve(model, tee=True)

# Display the results
print("\n===========================Results============================")
print(f"x1 = {model.x1.value}")
print(f"x2 = {model.x2.value}")
print(f"Objective value = {model.obj.expr()}")

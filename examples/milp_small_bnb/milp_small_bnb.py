# //
# //     MINOTAUR -- It's only 1/2 bull
# //
# //     (C)opyright 2008--2010 The MINOTAUR Team
# //

# // Contributer: Sumanto Kar(jeetsumanto123@gmail.com)

############# Mixed Integer Linear Program #########
#var x1 >= -100 <= 10 ;
#var x2 >= -100 <= 10 ;
#var x3 >= -100 <= 10  integer ;
#maximize obj: + 5*x1 + 6*x2 + 7*x3  - 0;

#subject to cons0: 0 <= + 1*x1 - 2*x2 + 3*x3  <= 15;

#subject to cons1: 0 <= + 2*x1 + 1*x2  <= 10;

#####################################################

from pyomo.environ import *

# Create a model
model = ConcreteModel()

# Define the variables
model.x1 = Var(bounds=(-100, 10), within=Reals)  # Continuous variable x1
model.x2 = Var(bounds=(-100, 10), within=Reals)  # Continuous variable x2
model.x3 = Var(bounds=(-100, 10), within=Integers)  # Integer variable x3

# Define the objective function: Minimize 5*x1 + 6*x2 + 7*x3
model.obj = Objective(expr=5 * model.x1 + 6 * model.x2 + 7 * model.x3, sense=maximize)

# Define the constraints
model.constr1 = Constraint(expr=(model.x1 - 2 * model.x2 + 3 * model.x3 <= 15))
model.constr1_lower = Constraint(expr=(model.x1 - 2 * model.x2 + 3 * model.x3 >= 0))

model.constr2 = Constraint(expr=(2 * model.x1 + model.x2 <= 10))
model.constr2_lower = Constraint(expr=(2 * model.x1 + model.x2 >= 0))

# Create a solver
solver = SolverFactory("mbnb", executable='../../build/bin/mbnb') 

# Solve the model
results = solver.solve(model, tee=True)

# Print results
print("\n===========================After Solving============================")
print(f"x1 = {model.x1.value}")
print(f"x2 = {model.x2.value}")
print(f"x3 = {model.x3.value}")
print(f"Objective Value = {model.obj.expr()}")


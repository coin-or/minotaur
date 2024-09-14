#################################################################
# ======= Knapsack problem==============
# var x1 >= 0 <= 1  binary ;
# var x2 >= 0 <= 1  binary ;
# var x3 >= 0 <= 1  binary ;
# maximize obj: + 80*x1 + 20*x2 + 100*x3  - 0;

# subject to cons0: 0 <= + 30*x1 + 10*x2 + 40*x3  <= 40;

#################################################################

from pyomo.environ import *

# Create the model
model = ConcreteModel()

# Data: values, weights, and capacity
values = [80, 20, 100]  # Example item values
weights = [30, 10, 40]   # Example item weights
capacity = 40            # Total weight capacity
n_items = len(values)     # Number of items

# Define the binary decision variables (1 if item is selected, 0 otherwise)
model.x = Var(range(n_items), within=Binary)

# Objective: Maximize total value
model.obj = Objective(expr=sum(values[i] * model.x[i] for i in range(n_items)), sense=maximize)

# Constraint: Total weight must not exceed capacity
model.weight_constraint = Constraint(expr=sum(weights[i] * model.x[i] for i in range(n_items)) <= capacity)

# Create a solver
solver = SolverFactory("mbnb", executable='../../build/bin/mbnb') 

# Solve the model
solver.solve(model, tee=True)

# Output the results
print("\n===========================After Solving============================")
selected_items = []
for i in range(n_items):
    if model.x[i].value == 1:
        selected_items.append(i)
        print(f"Item {i+1} is selected: Value = {values[i]}, Weight = {weights[i]}")

# Objective value
print(f"Total Value of Selected Items = {model.obj.expr()}")

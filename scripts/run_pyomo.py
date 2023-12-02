from pyomo.environ import *
import glob
import os
import importlib
import sys


#Before executing code set pythonpath
#echo $PYTHONPATH
#export PYTHONPATH=$PYTHONPATH:input_directory
#echo $PYTHONPATH


## SET THESE PARAMETERS BEFORE EVERY RUN
solverpath = "/home/mohan/minotaur/build/bin/mglob"
solver_options = {'--bnb_time_limit': 600}
input_directory = "/home/mohan/Downloads/minlplib/py1"
output_directory = "/home/mohan/Downloads/out"
echo_out = False


## DONT EDIT BELOW THIS

# List of Pyomo model files
model_files = glob.glob(os.path.join(input_directory, "*.py"))
print("Solving",len(model_files))


for model_file in model_files:
	# Extract the module name and class name from the file
	infile = os.path.splitext(os.path.basename(model_file))[0]
	print(infile)
	try:
		# Import the Pyomo model class
		model = importlib.import_module(infile)
	except ModuleNotFoundError as e:
		print(f"Error importing module {infile}: {e}")
		sys.exit(1)

	solver = SolverFactory("mglob", executable=solverpath)

	for option, value in solver_options.items():
		solver.options[option] = value

	# Extract instance name from the model file
	instance_name = os.path.splitext(os.path.basename(model_file))[0]
	output_file = os.path.join(output_directory, f"{instance_name}.out")
    
	try:
		result = solver.solve(model.m, tee=echo_out)
		with open(output_file, "w") as f:
			f.write("**************************************************************")
			f.write(f"\nResults for {model_file} : ")

			if result.solver.status == SolverStatus.ok:
				f.write("Solver terminated successfully.")
			elif result.solver.status == SolverStatus.aborted:
				f.write("Solver reached the time limit.")
			else:
				f.write("Solver did not terminate successfully")

			f.write("\nSolver status: {}".format(result.solver.status))
			f.write("\nBest solution value obtained before the time limit: {}".format(model.m.obj()))
			f.write("\nSolver termination condition: {}".format(result.solver.termination_condition))
			f.write("\nSolver time: {}".format(result.solver.time))
			f.write("\nBest bound: {}".format(result.problem.lower_bound))
			f.close()
	except Exception as e:
		print("Error solving", instance_name, ". Could not load results.")



import cplex

prob = cplex.Cplex("st_cqpjk1.lp")
#prob.objective.set_linear("objvar", 0.0)

for i in range(4):
    prob.objective.set_linear(i,1.0)
    prob.set_log_stream(None)
    prob.set_error_stream(None)
    prob.set_warning_stream(None)
    prob.set_results_stream(None)
    prob.solve()
    sol = prob.solution
    print(i, "min", sol.get_objective_value())
    prob.objective.set_linear(i,-1.0)
    prob.set_log_stream(None)
    prob.set_error_stream(None)
    prob.set_warning_stream(None)
    prob.set_results_stream(None)
    prob.solve()
    sol = prob.solution
    print(i, "max", -sol.get_objective_value())
    prob.objective.set_linear(i,0.0)

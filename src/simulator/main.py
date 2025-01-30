from pathlib import Path

from src.simulator.simulator import Simulator
from src.simulator.solver import Solver

if __name__ == "__main__":
    simulator = Simulator(Path("mazes/maze15x10_1.png"), (15,10), (7,9), (7,0))
    solver = Solver(simulator)
    simulator.solver_func = solver.solve_procedural
    simulator.display_maze()

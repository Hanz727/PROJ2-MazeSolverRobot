from pathlib import Path

from src.simulator.constants import SIMULATOR_MANUAL_MODE, SIMULATOR_PROCEDURAL_SOLVE, SIMULATOR_FULL_SOLVE
from src.simulator.simulator import Simulator
from src.simulator.solver import Solver

if __name__ == "__main__":
    simulator = Simulator(Path("mazes/maze15x10_1.png"), (15,10), (7,9), (7,0))
    solver = Solver(simulator)

    # change mode here
    mode = SIMULATOR_PROCEDURAL_SOLVE

    if mode == SIMULATOR_PROCEDURAL_SOLVE:
        simulator.solver_func = solver.solve_procedural

    if mode == SIMULATOR_FULL_SOLVE:
        solver.solve()

    simulator.display_maze()

from pathlib import Path

from src.simulator.simulator import Simulator
from src.simulator.solver import Solver

if __name__ == "__main__":
    simulator = Simulator(Path("mazes/maze7x5_1.png"), (7,5), (3,4), (3,0))
    solver = Solver(simulator)

    simulator.simulate_ultrasonic(3,4)
    simulator.flood_fill(3,0)
    simulator.draw_flood_fill_distances()

    # simulator.simulate_ultrasonic(8,8)
    # simulator.simulate_ultrasonic(5,8)
    # simulator.simulate_ultrasonic(5,9)
    simulator.display_maze()

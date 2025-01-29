from pathlib import Path

from src.simulator.simulator import Simulator

if __name__ == "__main__":
    simulator = Simulator(Path("mazes/maze10x10_1.png"), (10,10))
    simulator.simulate_ultrasonic(5,9)
    #simulator.simulate_ultrasonic(7,8)
    #simulator.simulate_ultrasonic(8,8)
    # simulator.simulate_ultrasonic(5,8)
    # simulator.simulate_ultrasonic(5,9)
    simulator.draw_wall_info((0,0,255), 2)
    simulator.display_maze()
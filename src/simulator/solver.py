from src.simulator.simulator import Simulator

class Solver:
    def __init__(self, simulator: Simulator):
        self.__simulator = simulator

        self.__start_pos = simulator.get_start_pos()
        self.__end_pos = simulator.get_end_pos()

        self.__grid_size = simulator.get_grid_size()

        self.__pos: list[int] = [*self.__start_pos]
        self.__move_count = 0

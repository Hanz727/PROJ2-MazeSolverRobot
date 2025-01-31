from src.simulator.simulator import Simulator

class Solver:
    def __init__(self, simulator: Simulator):
        self.__simulator = simulator

        self.__pos: list[int] = [*self.__simulator.start_pos]
        self.__last_pos: list[int] = self.__pos
        self.__move_count: int = 0

        self.__simulator.move(*self.__pos)

    def move(self):
        moves = self.__simulator.get_possible_moves(*self.__pos)
        if tuple(self.__last_pos) in moves:
            moves.remove(tuple(self.__last_pos))
            moves.append(tuple(self.__last_pos))
        best_move = []
        best_move_dist = 9999
        for move in moves:
            dist = self.__simulator.distance_grid[move[0]][move[1]]

            if dist < best_move_dist:
                best_move_dist = dist
                best_move = move

        self.__simulator.move(*best_move)
        self.__move_count += 1
        self.__last_pos = self.__pos
        self.__pos = best_move

    def solve(self):
        while True:
            walled_pos = self.__simulator.maze_grid_to_walled_grid(*self.__pos)
            if self.__simulator.walled_grid[walled_pos[0]][walled_pos[1]] == 3:
                break

            self.move()
        print("moves: ", self.__move_count)

    def solve_procedural(self) -> bool:
        walled_pos = self.__simulator.maze_grid_to_walled_grid(*self.__pos)
        if self.__simulator.walled_grid[walled_pos[0]][walled_pos[1]] == 3:
            return True
        self.move()
        return False

import math
from pathlib import Path
from collections import deque
import cv2
import numpy as np

from src.simulator.constants import MAXIMUM_IMAGE_WIDTH


class Simulator:
    def __init__(self, maze_path: Path, grid_size: (int, int), start_pos: (int, int), end_pos: (int, int)):
        self.__original_img = None
        self.__maze_img = None
        self.__maze_img_w = 0 # px
        self.__maze_img_h = 0 # px

        self.grid_size = grid_size                    # x, y idx
        self.cell_shape: tuple[int, int] = (0, 0)      # WIDTH, HEIGHT px

        self.walled_grid = []
        self.walled_grid_size = (grid_size[0] * 2 + 1, grid_size[1] * 2 + 1)

        self.visited_points = []

        self.distance_grid = [[-1 for _ in range(self.grid_size[1])] for _ in range(self.grid_size[0])]

        self.__directions = ((-1, 0), (1, 0), (0, -1), (0, 1)) # LEFT, RIGHT, UP, DOWN

        self.start_pos = start_pos
        self.end_pos = end_pos

        self.solver_func = None
        self.pos = start_pos

        self.__load_maze(maze_path, start_pos, end_pos)
        self.__setup_walled_grid()

    def flood_fill(self, start_x: int, start_y: int):
        self.distance_grid = [[-1 for _ in range(self.grid_size[1])] for _ in range(self.grid_size[0])]

        queue = deque([(start_x, start_y, 0)])  # (x, y, distance)
        self.distance_grid[start_x][start_y] = 0
        while queue:
            grid_x, grid_y, dist = queue.popleft()

            # Explore neighbors
            for dir_ in self.__directions:
                new_x, new_y = grid_x + dir_[0], grid_y + dir_[1]

                # Skip out-of-bound positions
                if new_x < 0 or new_y < 0 or new_x >= self.grid_size[0] or new_y >= self.grid_size[1]:
                    continue

                # Skip already visited positions
                if self.distance_grid[new_x][new_y] != -1:
                    continue

                walled_grid_pos = self.maze_grid_to_walled_grid(new_x, new_y)
                wall_pos = [a-b for a,b in zip(walled_grid_pos, dir_)]

                # Skip if the new position is blocked by a wall
                if self.walled_grid[wall_pos[0]][wall_pos[1]] == 1:
                    continue

                # Set the distance for this position
                self.distance_grid[new_x][new_y] = dist + 1

                # Add the new position to the queue
                queue.append((new_x, new_y, dist + 1))

    def __setup_walled_grid(self):
        for x in range(self.walled_grid_size[0]):
            self.walled_grid.append([])
            for y in range(self.walled_grid_size[1]):
                wall_state = 0
                if y == 0:
                    wall_state = 1
                if x == 0:
                    wall_state = 1
                if x == self.walled_grid_size[0]-1:
                    wall_state = 1
                if y == self.walled_grid_size[1]-1:
                    wall_state = 1

                self.walled_grid[x].append(wall_state)


        w_start_pos = self.maze_grid_to_walled_grid(*self.start_pos)
        w_end_pos = self.maze_grid_to_walled_grid(*self.end_pos)

        self.walled_grid[w_start_pos[0]][w_start_pos[1]] = 2
        self.walled_grid[w_end_pos[0]][w_end_pos[1]] = 3

    def __load_maze(self, maze_path: Path, start_pos: (int, int), end_pos: (int, int)):
        self.__maze_img = cv2.imread(maze_path.as_posix())
        if self.__maze_img is None:
            print("Error: unable to read image")
            return

        aspect_ratio = self.__maze_img.shape[1] / self.__maze_img.shape[0]
        scaled_y = MAXIMUM_IMAGE_WIDTH
        scaled_x = int(aspect_ratio * scaled_y)

        self.__maze_img = cv2.resize(self.__maze_img,
                                     (scaled_x, scaled_y),
                                     interpolation=cv2.INTER_NEAREST)
        self.__original_img = self.__maze_img.copy()

        self.__maze_img_w, self.__maze_img_h = self.__maze_img.shape[1], self.__maze_img.shape[0]

        self.cell_shape = (self.__maze_img_w / self.grid_size[0],
                           self.__maze_img_h / self.grid_size[1])

    def grid_to_pixel(self, x: int, y: int) -> tuple[int, int]:
        return int((x+0.5) * self.cell_shape[0]), int((y + 0.5) * self.cell_shape[1])

    def pixel_to_grid(self, x: int, y: int) -> tuple[int, int]:
        return (int(math.floor(x / self.__maze_img_w * self.grid_size[0])),
                int(math.floor(y / self.__maze_img_h * self.grid_size[1])))

    def maze_grid_to_walled_grid(self, grid_x: int, grid_y: int) -> (int, int):
        return grid_x * 2 + 1, grid_y * 2 + 1

    def walled_grid_to_maze_grid(self, grid_x: int, grid_y: int) -> (int, int):
        return int((grid_x - 1) / 2.), int((grid_y - 1) / 2.)

    def __mark_walls(self, pos_px: (int, int), sensor_values: (int, int, int, int)) -> None:
        # West / Left
        if sensor_values[0]:
            w_grid = self.maze_grid_to_walled_grid(*self.pixel_to_grid(pos_px[0] - sensor_values[0], pos_px[1]))
            self.walled_grid[w_grid[0] - 1][w_grid[1]] = 1

        # East / Right
        if sensor_values[1]:
            w_grid = self.maze_grid_to_walled_grid(*self.pixel_to_grid(pos_px[0] + sensor_values[1], pos_px[1]))
            self.walled_grid[w_grid[0] + 1][w_grid[1]] = 1

        # North / Up
        if sensor_values[2]:
            w_grid = self.maze_grid_to_walled_grid(*self.pixel_to_grid(pos_px[0], pos_px[1] - sensor_values[2]))
            self.walled_grid[w_grid[0]][w_grid[1] - 1] = 1

        # South / Down
        if sensor_values[3]:
            w_grid = self.maze_grid_to_walled_grid(*self.pixel_to_grid(pos_px[0], pos_px[1] + sensor_values[3]))
            self.walled_grid[w_grid[0]][w_grid[1] + 1] = 1

    def simulate_ultrasonic(self, grid_x: int, grid_y: int) -> (int, int, int, int):
        xp, yp = self.grid_to_pixel(grid_x, grid_y)
        ret = [0,0,0,0]

        # West / Left
        for xl in range(xp):
            if not any(self.__maze_img[yp, xp - xl]):
                ret[0] = xl - 2
                break

        # East / Right
        for xr in range(self.__maze_img_w - xp):
            if not any(self.__maze_img[yp, xp + xr]):
                ret[1] = xr - 2
                break

        # North / Up
        for yt in range(yp):
            if not any(self.__maze_img[yp - yt, xp]):
                ret[2] = yt - 2
                break

        # South / Down
        for yb in range(self.__maze_img_h - yp):
            if not any(self.__maze_img[yp + yb, xp]):
                ret[3] = yb - 2
                break

        self.__mark_walls((xp, yp), ret)
        return ret

    def print_walled_maze(self):
        print_grid = np.array(self.walled_grid).T
        for y in range(self.walled_grid_size[1]):
            print(print_grid[y])

    def print_dist_grid(self):
        print_grid = np.array(self.distance_grid).T
        for y in range(self.grid_size[1]):
            print(print_grid[y])

    def draw_flood_fill_distances(self):
        for x in range(self.grid_size[0]):
            for y in range(self.grid_size[1]):
                color = (0,0,0)
                if (x,y) in self.visited_points:
                    color = (0,150,0)
                cv2.putText(self.__maze_img, str(self.distance_grid[x][y]), self.grid_to_pixel(x, y),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, color, 1, cv2.LINE_AA)

    def draw_pos(self, grid_x, grid_y):
        pos = self.grid_to_pixel(grid_x, grid_y)
        cv2.circle(self.__maze_img, pos, 5, (180,0,0), -1)

    def draw_marked_walls(self, color: (int,int,int) = (0,0,255), thickness: int = 2):
        for x in range(self.grid_size[0]):
            for y in range(self.grid_size[1]):
                walled_grid_pos = self.maze_grid_to_walled_grid(x, y)
                x_px, y_px = self.grid_to_pixel(x, y)

                # Left
                if self.walled_grid[walled_grid_pos[0] - 1][walled_grid_pos[1]] == 1:
                    cv2.line(self.__maze_img,
                             (int(x_px - self.cell_shape[0] * 0.33), int(y_px - self.cell_shape[1] * 0.33)),
                             (int(x_px - self.cell_shape[0] * 0.33), int(y_px + self.cell_shape[1] * 0.33)),
                             color, thickness, cv2.LINE_AA)

                # Right
                if self.walled_grid[walled_grid_pos[0] + 1][walled_grid_pos[1]] == 1:
                    cv2.line(self.__maze_img,
                             (int(x_px + self.cell_shape[0] * 0.33), int(y_px - self.cell_shape[1] * 0.33)),
                             (int(x_px + self.cell_shape[0] * 0.33), int(y_px + self.cell_shape[1] * 0.33)),
                             color, thickness, cv2.LINE_AA)

                # Top
                if self.walled_grid[walled_grid_pos[0]][walled_grid_pos[1] - 1] == 1:
                    cv2.line(self.__maze_img,
                             (int(x_px - self.cell_shape[0] * 0.33), int(y_px - self.cell_shape[1] * 0.33)),
                             (int(x_px + self.cell_shape[0] * 0.33), int(y_px - self.cell_shape[1] * 0.33)),
                             color, thickness, cv2.LINE_AA)

                # Bottom
                if self.walled_grid[walled_grid_pos[0]][walled_grid_pos[1] + 1] == 1:
                    cv2.line(self.__maze_img,
                             (int(x_px - self.cell_shape[0] * 0.33), int(y_px + self.cell_shape[1] * 0.33)),
                             (int(x_px + self.cell_shape[0] * 0.33), int(y_px + self.cell_shape[1] * 0.33)),
                             color, thickness, cv2.LINE_AA)

    def get_possible_moves(self, grid_x, grid_y) -> list[(int, int)]:
        moves = []
        walled_grid_pos = self.maze_grid_to_walled_grid(grid_x, grid_y)

        for dir_ in self.__directions:
            wall_pos = walled_grid_pos[0] + dir_[0], walled_grid_pos[1] + dir_[1]
            if self.walled_grid[wall_pos[0]][wall_pos[1]] != 1:
                next_move_pos = wall_pos[0] + dir_[0], wall_pos[1] + dir_[1] # Move once more in same dir
                moves.append(self.walled_grid_to_maze_grid(*next_move_pos))

        return moves

    def move(self, grid_x, grid_y):
        self.visited_points.append((grid_x, grid_y))
        self.simulate_ultrasonic(grid_x, grid_y)
        self.flood_fill(*self.end_pos)
        self.pos = (grid_x, grid_y)

    def on_click(self, event, x, y, flags, param):
        if event == cv2.EVENT_LBUTTONDOWN:
            pos = self.pixel_to_grid(x, y)
            self.move(*pos)

    def display_maze(self) -> None:
        cv2.namedWindow("Maze")
        cv2.setMouseCallback("Maze", self.on_click)
        while True:
            self.__maze_img = self.__original_img.copy()
            self.draw_pos(*self.pos)
            self.draw_flood_fill_distances()
            self.draw_marked_walls()

            cv2.imshow("Maze", self.__maze_img)
            if self.solver_func:
                key = cv2.waitKey(0)
                if self.solver_func():
                    print("Solved")
            else:
                key = cv2.waitKey(1)

            if key == 27: # ESC
                break
        cv2.destroyAllWindows()

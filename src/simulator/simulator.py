import math
from pathlib import Path
from collections import deque
import cv2
import numpy as np

from src.simulator.constants import MAXIMUM_IMAGE_WIDTH


class Simulator:
    def __init__(self, maze_path: Path, grid_size: (int, int), start_pos: (int, int), end_pos: (int, int)):
        self.__maze_img = None
        self.__maze_img_w = 0 # px
        self.__maze_img_h = 0 # px

        self.__grid_size = grid_size                    # x, y idx
        self.__cell_shape: tuple[int, int] = (0,0)      # WIDTH, HEIGHT px

        self.__walled_grid = []
        self.__walled_grid_size = (grid_size[0]*2+1, grid_size[1]*2+1)

        self.__distance_grid = []

        self.__directions = ((-1, 0), (1, 0), (0, -1), (0, 1)) # LEFT, RIGHT, UP, DOWN

        self.__start_pos = start_pos
        self.__end_pos = end_pos

        self.__load_maze(maze_path, start_pos, end_pos)
        self.__setup_walled_grid()

    def get_grid_size(self):
        return self.__grid_size

    def get_start_pos(self):
        return self.__start_pos

    def get_end_pos(self):
        return self.__end_pos

    def flood_fill(self, start_x: int, start_y: int):
        self.__distance_grid = [[-1 for _ in range(self.__grid_size[1])] for _ in range(self.__grid_size[0])]

        queue = deque([(start_x, start_y, 0)])  # (x, y, distance)
        self.__distance_grid[start_x][start_y] = 0
        while queue:
            grid_x, grid_y, dist = queue.popleft()

            # Explore neighbors
            for dir_ in self.__directions:
                new_x, new_y = grid_x + dir_[0], grid_y + dir_[1]

                # Skip out-of-bound positions
                if new_x < 0 or new_y < 0 or new_x >= self.__grid_size[0] or new_y >= self.__grid_size[1]:
                    continue

                # Skip already visited positions
                if self.__distance_grid[new_x][new_y] != -1:
                    continue

                walled_grid_pos = self.__maze_grid_to_walled_grid(new_x, new_y)
                wall_pos = [a-b for a,b in zip(walled_grid_pos, dir_)]

                # Skip if the new position is blocked by a wall
                if self.__walled_grid[wall_pos[0]][wall_pos[1]] == 1:
                    continue

                # Set the distance for this position
                self.__distance_grid[new_x][new_y] = dist + 1

                # Add the new position to the queue
                queue.append((new_x, new_y, dist + 1))

    def __setup_walled_grid(self):
        for x in range(self.__walled_grid_size[0]):
            self.__walled_grid.append([])
            for y in range(self.__walled_grid_size[1]):
                wall_state = 0
                if y == 0:
                    wall_state = 1
                if x == 0:
                    wall_state = 1
                if x == self.__walled_grid_size[0]-1:
                    wall_state = 1
                if y == self.__walled_grid_size[1]-1:
                    wall_state = 1

                self.__walled_grid[x].append(wall_state)


        w_start_pos = self.__maze_grid_to_walled_grid(*self.__start_pos)
        w_end_pos = self.__maze_grid_to_walled_grid(*self.__end_pos)

        self.__walled_grid[w_start_pos[0]][w_start_pos[1]] = 2
        self.__walled_grid[w_end_pos[0]][w_end_pos[1]] = 3

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
        self.__maze_img_w, self.__maze_img_h = self.__maze_img.shape[1], self.__maze_img.shape[0]

        self.__cell_shape = (self.__maze_img_w / self.__grid_size[0],
                             self.__maze_img_h / self.__grid_size[1])

    def __grid_to_pixel(self, x: int, y: int) -> tuple[int, int]:
        return int((x+0.5) * self.__cell_shape[0]), int((y+0.5) * self.__cell_shape[1])

    def __pixel_to_grid(self, x: int, y: int) -> tuple[int, int]:
        return (int(math.floor(x/self.__maze_img_w*self.__grid_size[0])),
                int(math.floor(y/self.__maze_img_h*self.__grid_size[1])))

    def __maze_grid_to_walled_grid(self, grid_x: int, grid_y: int) -> (int, int):
        return grid_x * 2 + 1, grid_y * 2 + 1

    def __mark_walls(self, pos_px: (int, int), sensor_values: (int, int, int, int)) -> None:
        # West / Left
        if sensor_values[0]:
            w_grid = self.__maze_grid_to_walled_grid(*self.__pixel_to_grid(pos_px[0] - sensor_values[0], pos_px[1]))
            self.__walled_grid[w_grid[0]-1][w_grid[1]] = 1

        # East / Right
        if sensor_values[1]:
            w_grid = self.__maze_grid_to_walled_grid(*self.__pixel_to_grid(pos_px[0] + sensor_values[1], pos_px[1]))
            self.__walled_grid[w_grid[0]+1][w_grid[1]] = 1

        # North / Up
        if sensor_values[2]:
            w_grid = self.__maze_grid_to_walled_grid(*self.__pixel_to_grid(pos_px[0], pos_px[1] - sensor_values[2]))
            self.__walled_grid[w_grid[0]][w_grid[1]-1] = 1

        # South / Down
        if sensor_values[3]:
            w_grid = self.__maze_grid_to_walled_grid(*self.__pixel_to_grid(pos_px[0], pos_px[1] + sensor_values[3]))
            self.__walled_grid[w_grid[0]][w_grid[1] + 1] = 1

    def simulate_ultrasonic(self, grid_x: int, grid_y: int) -> (int, int, int, int):
        xp, yp = self.__grid_to_pixel(grid_x, grid_y)
        ret = [0,0,0,0]

        # West / Left
        for xl in range(xp):
            if not any(self.__maze_img[yp, xp - xl]):
                ret[0] = xl
                break

        # East / Right
        for xr in range(self.__maze_img_w - xp):
            if not any(self.__maze_img[yp, xp + xr]):
                ret[1] = xr
                break

        # North / Up
        for yt in range(yp):
            if not any(self.__maze_img[yp - yt, xp]):
                ret[2] = yt
                break

        # South / Down
        for yb in range(self.__maze_img_h - yp):
            if not any(self.__maze_img[yp + yb, xp]):
                ret[3] = yb
                break

        self.__mark_walls((xp, yp), ret)
        return ret

    def print_walled_maze(self):
        print_grid = np.array(self.__walled_grid).T
        for y in range(self.__walled_grid_size[1]):
            print(print_grid[y])

    def print_dist_grid(self):
        print_grid = np.array(self.__distance_grid).T
        for y in range(self.__grid_size[1]):
            print(print_grid[y])

    def draw_flood_fill_distances(self):
        for x in range(self.__grid_size[0]):
            for y in range(self.__grid_size[1]):
                cv2.putText(self.__maze_img, str(self.__distance_grid[x][y]), self.__grid_to_pixel(x,y),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,0,0),1,cv2.LINE_AA)

    def draw_marked_walls(self):
        for x in range(self.__walled_grid_size[0]):
            for y in range(self.__walled_grid_size[1]):
                ...

    def display_maze(self) -> None:
        cv2.imshow("Maze", self.__maze_img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

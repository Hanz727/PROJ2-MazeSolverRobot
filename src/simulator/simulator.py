import math
from pathlib import Path

import cv2

from src.simulator.constants import MAXIMUM_IMAGE_WIDTH
from src.simulator.contracts import WallData


class Simulator:
    def __init__(self, maze_path: Path | None = None, grid_size: (int, int) = (0, 0),
                 start_pos: (int, int) = (0,0), end_pos: (int, int) = (0, 0)):
        self.__maze_img = None
        self.__maze_img_w = 0 # px
        self.__maze_img_h = 0 # px

        self.__grid_size = grid_size                    # x, y idx
        self.__cell_shape: tuple[int, int] = (0,0)      # WIDTH, HEIGHT px

        self.__sensors = (True, True, True, True) # Enable or disable sensors: left, right, front (top), back (bottom)

        self.__wall_info: dict[(int, int), WallData] = {}

        self.__pos = [0,0]

        if maze_path:
            self.load_maze(maze_path, start_pos, end_pos)

    def load_maze(self, maze_path: Path, start_pos: (int, int), end_pos: (int, int)):
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

        self.__setup_wall_info(start_pos, end_pos)

    def __setup_wall_info(self, start_pos: (int, int), end_pos: (int, int)):
        for x in range(self.__grid_size[0]):
            for y in range(self.__grid_size[1]):
                l = True if x == 0 else 0
                r = True if x == self.__grid_size[0] - 1 else 0
                t = True if y == 0 else 0
                b = True if y == self.__grid_size[1] - 1 else 0
                self.__wall_info[(x, y)] = WallData(l,r,t,b)

                if (x,y) == start_pos:
                    self.__wall_info[(x,y)].start = True

                if (x,y) == end_pos:
                    self.__wall_info[(x,y)].end = True

    def __grid_to_pixel(self, x: int, y: int) -> tuple[int, int]:
        return int((x+0.5) * self.__cell_shape[0]), int((y+0.5) * self.__cell_shape[1])

    def __pixel_to_grid(self, x: int, y: int) -> tuple[int, int]:
        return (int(math.floor(x/self.__maze_img_w*self.__grid_size[0])),
                int(math.floor(y/self.__maze_img_h*self.__grid_size[1])))

    def draw_grid_points(self, color: (int, int, int ) = (0,0,255)):
        for x in range(self.__grid_size[0]):
            for y in range(self.__grid_size[1]):
                pos = self.__grid_to_pixel(x, y)
                cv2.circle(self.__maze_img, (pos[0], pos[1]), 3, color, -1)

    def draw_wall_info(self, color: (int, int, int ) = (255,255,0), thickness: int = 1):
        for x in range(self.__grid_size[0]):
            for y in range(self.__grid_size[1]):
                pos = self.__grid_to_pixel(x, y)

                cell_info = self.__wall_info[(x,y)]
                # LEFT
                if cell_info.left:
                    cv2.arrowedLine(self.__maze_img, pos, (int(pos[0] - 0.33 * self.__cell_shape[0]), pos[1]),
                                    color, thickness)

                # RIGHT
                if cell_info.right:
                    cv2.arrowedLine(self.__maze_img, pos, (int(pos[0] + 0.33 * self.__cell_shape[0]), pos[1]),
                                    color, thickness)

                # TOP
                if cell_info.top:
                    cv2.arrowedLine(self.__maze_img, pos, (pos[0], int(pos[1] - 0.33 * self.__cell_shape[1])),
                                    color, thickness)

                # BOTTOM
                if cell_info.bottom:
                    cv2.arrowedLine(self.__maze_img, pos, (pos[0], int(pos[1] + 0.33 * self.__cell_shape[1])),
                                    color, thickness)


                if self.__wall_info[(x, y)].scanned:
                    cv2.circle(self.__maze_img, (pos[0], pos[1]), 5, (0,255,0), -1)

    def display_maze(self):
        cv2.imshow("Maze", self.__maze_img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

    def draw_ultrasonic(self, grid_x: int, grid_y: int, sensor_values: (int, int, int, int), color: (int, int, int) = (255,0,0)):
        xp, yp = self.__grid_to_pixel(grid_x, grid_y)

        # LEFT
        if sensor_values[0]:
            cv2.circle(self.__maze_img, (xp - sensor_values[0],yp), 3, color, -1)

        # RIGHT
        if sensor_values[1]:
            cv2.circle(self.__maze_img, (xp + sensor_values[1], yp), 3, color, -1)

        # TOP
        if sensor_values[2]:
            cv2.circle(self.__maze_img, (xp, yp - sensor_values[2]), 3, color, -1)

        # BOTTOM
        if sensor_values[3]:
            cv2.circle(self.__maze_img, (xp, yp + sensor_values[3]), 3, color, -1)



    def set_sensors(self, sensors: (bool, bool, bool, bool)):
        self.__sensors = sensors

    def __update_wall_info(self, x_px, y_px, sensor_values: (int, int, int, int)):
        # LEFT
        if sensor_values[0]:
            self.__wall_info[self.__pixel_to_grid(x_px - sensor_values[0], y_px)].left = 1

        # RIGHT
        if sensor_values[1]:
            self.__wall_info[self.__pixel_to_grid(x_px + sensor_values[1], y_px)].right = 1

        # TOP
        if sensor_values[2]:
            self.__wall_info[self.__pixel_to_grid(x_px, y_px - sensor_values[2])].top = 1

        # BOTTOM
        if sensor_values[3]:
            self.__wall_info[self.__pixel_to_grid(x_px, y_px + sensor_values[3])].bottom = 1

    def simulate_ultrasonic(self, grid_x: int, grid_y: int):
        ret = [0,0,0,0]
        xp, yp = self.__grid_to_pixel(grid_x, grid_y)

        self.__wall_info[(grid_x, grid_y)].scanned = True
        self.__wall_info[(grid_x, grid_y)].visited = True

        # LEFT
        if self.__sensors[0]:
            for xl in range(xp):
                if not any(self.__maze_img[yp, xp - xl]):
                    ret[0] = xl
                    left = self.__pixel_to_grid(xp-xl, yp)
                    for x in range(left[0], grid_x):
                        self.__wall_info[(x, grid_y)].scanned = True
                    break

        # RIGHT
        if self.__sensors[1]:
            for xr in range(self.__maze_img_w - xp):
                if not any(self.__maze_img[yp, xp + xr]):
                    ret[1] = xr
                    right = self.__pixel_to_grid(xp+xr, yp)
                    for x in range(grid_x, right[0]+1):
                        self.__wall_info[(x, grid_y)].scanned = True
                    break

        # TOP
        if self.__sensors[2]:
            for yt in range(yp):
                if not any(self.__maze_img[yp - yt, xp]):
                    ret[2] = yt
                    top = self.__pixel_to_grid(xp, yp - yt)
                    for y in range(top[1], grid_y):
                        self.__wall_info[(grid_x, y)].scanned = True
                    break

        # BOTTOM
        if self.__sensors[3]:
            for yb in range(self.__maze_img_h - yp):
                if not any(self.__maze_img[yp + yb, xp]):
                    ret[3] = yb
                    bottom = self.__pixel_to_grid(xp, yp + yb)
                    for y in range(grid_y, bottom[1]+1):
                        self.__wall_info[(grid_x, y)].scanned = True
                    break

        self.__update_wall_info(xp, yp, ret)

        return ret

    def check_possible_moves(self, grid_x: int, grid_y: int) -> (int, int, int, int):
        ...

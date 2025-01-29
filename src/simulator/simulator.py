from pathlib import Path
import cv2

class Simulator:
    def __init__(self, maze_path: Path | None = None, grid_size: (int, int) = (0, 0)):
        self.__maze_img = None
        self.__maze_img_shape: tuple[int, int] = (0, 0) # WIDTH, HEIGHT px

        self.__grid_size = grid_size                    # x, y idx
        self.__cell_shape: tuple[int, int] = (0,0)      # WIDTH, HEIGHT px

        if maze_path:
            self.load_maze(maze_path)

    def load_maze(self, maze_path: Path):
        self.__maze_img = cv2.imread(maze_path.as_posix())
        if self.__maze_img is None:
            print("Error: unable to read image")
            return

        aspect_ratio = self.__maze_img.shape[1] / self.__maze_img.shape[0]
        scaled_y = 600
        scaled_x = int(aspect_ratio * scaled_y)

        self.__maze_img = cv2.resize(self.__maze_img,
                                     (scaled_x, scaled_y),
                                     interpolation=cv2.INTER_NEAREST)

        self.__maze_img_shape = self.__maze_img.shape[:2]
        self.__cell_shape = (self.__maze_img_shape[0] / self.__grid_size[0],
                             self.__maze_img_shape[1] / self.__grid_size[1])

    def display_maze(self):
        cv2.imshow("Maze", self.__maze_img)
        cv2.waitKey(0)
        cv2.destroyAllWindows()

    

if __name__ == '__main__':
    Simulator(Path("mazes/maze15x10_1.png"), (15,10)).display_maze()

# img = cv2.imread('imgs/maze.PNG')
# img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
#
# points = []
#
# grid_size = 10
# grid_size_px = 486
# block_size = grid_size_px / grid_size
#
# def point_to_pos(x: int, y: int) -> (int, int):
#     return int((x+1) * block_size - block_size / 2), int((y+1) * block_size - block_size / 2)
#
# for x in range(0,int(block_size)):
#     for y in range(0,int(block_size)):
#         points.append(point_to_pos(x, y))
#
# def project(x: int, y: int) -> (int, int, int, int):
#     ret = [0,0,0,0]
#
#     xp, yp = point_to_pos(x, y)
#
#     # LEFT
#     for xl in range(xp):
#         if not any(img[yp, xp - xl]):
#             ret[0] = xl
#             break
#
#     # RIGHT
#     for xr in range(grid_size_px - xp):
#         if not any(img[yp, xp + xr]):
#             ret[1] = xr
#             break
#
#     # TOP
#     for yt in range(yp):
#         if not any(img[yp - yt, xp]):
#             ret[2] = yt
#             break
#
#     for yb in range(grid_size_px - yp):
#         if not any(img[yp + yb, xp]):
#             ret[3] = yb
#             break
#
#     return ret
#
# def drawProjection(point: (int, int), proj: (int, int, int, int)):
#     pos = point_to_pos(*point)
#     color = (0,0,255)
#
#     if proj[0]:
#         cv2.circle(img, (pos[0] - proj[0], pos[1]), 4, color, -1) # LEFT
#
#     if proj[1]:
#         cv2.circle(img, (pos[0] + proj[1], pos[1]), 4, color, -1) # RIGHT
#
#     if proj[2]:
#         cv2.circle(img, (pos[0], pos[1] - proj[2]), 4, color, -1) # TOP
#
#     if proj[3]:
#         cv2.circle(img, (pos[0], pos[1] + proj[3]), 4, color, -1)
#
# drawProjection((5,9), project(5,9))
#
# for x in range(10):
#     for y in range(10):
#         drawProjection((x, y), project(x, y))
#
# #for point in points:
# #    cv2.circle(img, (int(point[0]), int(point[1])), 3, (0, 0, 255), -1)
#
# cv2.imshow("Image with Points", img)
# cv2.waitKey(0)
# cv2.destroyAllWindows()
#
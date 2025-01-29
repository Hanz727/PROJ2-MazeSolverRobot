from typing import Final

MAXIMUM_IMAGE_WIDTH = 600

class WallData:
    def __init__(self, l: bool = 0, r: bool = 0, t: bool = 0, b: bool = 0):
        self.left = l
        self.right = r
        self.top = t
        self.bottom = b
        self.visited = False
        self.start = False
        self.end = False
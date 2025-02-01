# PROJ2 Microcontroller overview
```
📦 PROJ2-MazeSolverRobot
├── 📂 src                   # Source code files
│   ├── 📂 arduino_sketch    # Everything that goes on the microcontroller
│   │   ├── arduino_sketch.ino   # (WIP) The main sketch that is to be uploaded to the microcontroller
│   │   ├── MazeSolver.h         # (WIP) Lib that solves the maze
│   │   ├── RangeFinder.h        # Lib that measures range from ultrasonic sensors
│   │   ├── RangeFinder.md       # RangeFinder documentation
│   │   └── ...                  # additional cpp/.h files 
│   └── 📂 simulator           # Python simulator for testing and visualizing path finding algorithms
│       ├── 📂 mazes           # Pngs for simulator
│       │   ├── maze_5x5_1.png   # Example 5 by 5 grid
│       │   └── ...
│       ├── main.py              # Program start point
│       ├── readme.md            # Simulator documentation
│       └── ...                  # Additional files for the simulator 
├── requirements.txt  # Project dependencies
├── README.md         # Project description
└── .gitignore        # Git ignore file
```
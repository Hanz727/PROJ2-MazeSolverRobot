# PROJ2 Microcontroller overview

## Installation

```
#Clone the repository (Optionally download ZIP if you don't want to use git)
git clone --recursive https://github.com/Hanz727/PROJ2-MazeSolverRobot.git

#Navigate to project dir
cd PROJ2-MazeSolverRobot
```
There are more steps for simulator and arduino_sketch. Follow their respective installation guide.

```
📦 PROJ2-MazeSolverRobot
├── 📂 src                       # Source code files
│   ├── 📂 arduino_sketch        # Everything that goes on the microcontroller
│   │   ├── arduino_sketch.ino   # (WIP) The main sketch that is to be uploaded to the microcontroller
│   │   ├── MazeSolver           # Lib that solves the maze
│   │   └── RangeFinder          # Lib that measures range from ultrasonic sensors
│   └── 📂 simulator             # Python simulator for testing and visualizing path finding algorithms
│       ├── 📂 mazes             # Pngs for simulator
│       │   ├── maze_5x5_1.png   # Example 5 by 5 grid
│       │   └── ...
│       ├── main.py              # Simulator start point
│       ├── readme.md            # Simulator documentation
│       └── ...                  # Additional files for the simulator 
├── requirements.txt  # Project dependencies
├── README.md         # Project description
└── .gitignore        # Git ignore file
```

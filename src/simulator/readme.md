# Simulator usage guide
The simulator is used to test path finding algorithms.

## Requirements
- **Python 3.12+** (Check with: `python --version`)
- **pip** (Check with `pip --version`)

## Installation
```sh
# Clone the repository (Optionally download ZIP if you don't want to use git)
git clone --recursive https://github.com/Hanz727/PROJ2-MazeSolverSimulator.git

# Navigate to project dir
cd PROJ2-MazeSolverSimulator

# Install dependencies
pip install -r requirements.txt
```

## Usage
1. Choose one of the mazes from ```src/simulator/mazes```.
2. In main.py: Adjust the following line with the correct path to img, grid size, start coordinates and end coordinates. Top left is (0,0)
```py 
simulator = Simulator(Path("src/simulator/mazes/maze15x10_1.png"), (15,10), (7,9), (7,0))
```
3. In main.py: Choose a mode by editing the following line:
```python
mode = SIMULATOR_PROCEDURAL_MODE
```
### modes
| Mode                          | Description                                                                                                                                                                                                    |
|-------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **SIMULATOR_MANUAL_MODE**     | Move the robot to any position with a mouse click, on each position change the ultrasonic sensors are simulated and walls are detected and stored.                                                             |
| **SIMULATOR_PROCEDURAL_MODE** | The maze is solved automatically with the algorithm specified by the `Solver.move()` function, press any button but `ESC` to move the next position.                                                           |
| **SIMULATOR_FULL_SOLVE_MODE** | The maze is solved by the `Solver.solve()` function using `Solver.move()` algorithm and the resulting amount of moves taken is printed out. Mouse input is allowed on the solved maze like in the manual mode. |

4. To exit the program press `ESC`.
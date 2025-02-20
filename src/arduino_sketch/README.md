# Arduino sketch usage and setup guide
This sketch will be uploaded to the microcontroller on the physical robot.

## Installation
```sh
#Clone the repository (Optionally download ZIP if you don't want to use git)
git clone --recursive https://github.com/Hanz727/PROJ2-MazeSolverRobot.git
```

### Automatic
1. Launch the installer, either bat or sh (for mac).
2. For .sh file you need to use the console command `./install_libs_mac.sh`.
3. If it says No Permission use `chmod +x install_libs_mac.sh` and repeat step 2.

### Manual

1. Go to /PROJ2-MazeSolverRobot/src/arduino_sketch/
2. Send MazeSolver and RangeFinder to .zip
3. arduino IDE -> File -> open -> arduino_sketch.ino
4. arduino IDE -> Sketch -> Include library -> add .ZIP library... -> MazeSolver.zip
5. Repeat step 4 for RangeFinder.zip

## TroubleShooting

When you update a .ZIP library, you must first remove it. Otherwise the following error occurs:
```
Error: 13 INTERNAL: Library install failed: moving extracted archive to destination dir: Library ... is already installed, but with a different version: ...@1.0.0
```

To remove a library head to Documents/Arduino/libraries and delete a folder with the name of the lib.

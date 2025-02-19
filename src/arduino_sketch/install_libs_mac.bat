#!/bin/bash

# Set paths
ARDUINO_LIB="$HOME/Library/Arduino15/libraries"
SOURCE_DIR="$(pwd)"

# List of libraries to update
LIBRARIES=("MazeSolver" "RangeFinder" "ShieldMotor")

echo "Updating Arduino libraries..."

# Loop through each library and replace it
for LIB in "${LIBRARIES[@]}"; do
    echo "Removing $LIB..."
    rm -rf "$ARDUINO_LIB/$LIB"
    
    echo "Copying $LIB..."
    cp -R "$SOURCE_DIR/$LIB" "$ARDUINO_LIB/$LIB"
done

echo "Done!"
read -p "Press Enter to exit..."

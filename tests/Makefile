CXX = g++
# DEBUG flags
CXXFLAGS = -Wall -Wextra -std=c++17 -I../src/arduino_sketch -I. -g -O0

TARGET = debug/testMazeSolver

SRCS = testMazeSolver.cpp runTests.cpp ../src/arduino_sketch/MazeSolver.cpp
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Compile the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Compile object files
# Fix: Make sure object files are placed in the same directory as source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

../src/arduino_sketch/%.o: ../src/arduino_sketch/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean compiled files
clean:
	rm -f $(OBJS) $(TARGET)

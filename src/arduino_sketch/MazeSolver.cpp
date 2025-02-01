#include "MazeSolver.h"
#include "math.h"
#include "platformDefinitions.h"
#include "FixedDeque.h"

#ifdef WINDOWS
    #include <iostream>
#endif

MazeSolver::MazeSolver(
    const double wallWidth,
    const double cellWidth,
    const double cellHeight,
    const uint8_t mazeWidth,
    const uint8_t mazeHeight,
    const vec2<int> startPos,
    const vec2<int> endPos
) :
    m_wallWidth(wallWidth),
    m_cellWidth(cellWidth),
    m_cellHeight(cellHeight),
    m_MazeWidth(mazeWidth),
    m_MazeHeight(mazeHeight),
    m_startPos(startPos),
    m_endPos(endPos),
    m_MazeWidthEx(mazeWidth * 2 + 1),
    m_MazeHeightEx(mazeHeight * 2 + 1)
{
    clearDistanceMatrix();
    clearWallMatrix();
}

MazeSolver::~MazeSolver() {
}

void MazeSolver::clearDistanceMatrix() {
    for (int x = 0; x < m_MazeWidth; x++) {
        for (int y = 0; y < m_MazeHeight; y++) {
            m_distanceMatrix[x][y] = -1;
        }
    }
}

void MazeSolver::clearWallMatrix() {
    for (int x = 0; x < m_MazeWidthEx; x++) {
        for (int y = 0; y < m_MazeHeightEx; y++) {
            bool wallState = 0;
            if (x == 0 || y == 0)
                wallState = 1;

            if (x == m_MazeWidthEx - 1 || y == m_MazeHeightEx - 1)
                wallState = 1;

            m_wallMatrix[x][y] = wallState;
        }
    }
}


vec2<double> MazeSolver::posToCm(const vec2<double>& pos) const {
    return vec2<double>(
        (m_cellWidth + m_wallWidth) * pos.x + (m_wallWidth + m_cellWidth / 2.),
        (m_cellHeight + m_wallWidth) * pos.y + (m_wallWidth + m_cellHeight / 2.)
    );
}

vec2<double> MazeSolver::cmToPos(const vec2<double>& cm) const {
    return vec2<double>(
        (cm.x - m_wallWidth - m_cellWidth / 2.) / (m_cellWidth + m_wallWidth),
        (cm.y - m_wallWidth - m_cellHeight / 2.) / (m_cellHeight + m_wallWidth)
    );
}

vec2<int> MazeSolver::posToPosEx(const vec2<double>& pos) const {
    return vec2<int>(
        round(pos.x * 2. + 1.),
        round(pos.y * 2. + 1.)
    );
}

vec2<double> MazeSolver::posExToPos(const vec2<int>& posEx) const {
    return vec2<int>(
        (posEx.x - 1.) / 2.,
        (posEx.y - 1.) / 2.
    );
}


// Parameters:
// - pos: The current position in the maze grid, represented as a vec2<double>. The more precise the position the better.
// - distance: The distance to the wall in centimeters (double). This value is used
//   to calculate the position of the wall based on the direction.
// - dir: The direction from the current position in which the wall is located, 
//   represented by the CompassDir enum (e.g., North, South, East, West). Where north goes towards y = 0 and West goes to x = 0
// 
// Measurement tolerance of ±4.7 cm. If exceeded everything breaks.
void MazeSolver::markWall(const vec2 <double>& pos, const double distance, CompassDir dir) {
    vec2<double> wallPosCm = posToCm(pos) + (vec2<double>{ distance,distance }*m_directions[(int)dir]);
    vec2<int> wallPosEx = posToPosEx(cmToPos(wallPosCm));

    // walls are only on even spots
    if (!(wallPosEx.x % 2 == 0 || wallPosEx.y % 2 == 0))
        return;

    // If the wall wasn't detected before
    if (m_wallMatrix[wallPosEx.x][wallPosEx.y] == 1)
        return;

    m_wallMatrix[wallPosEx.x][wallPosEx.y] = 1;
    floodFill(m_endPos);
}

void MazeSolver::floodFill(const vec2<int>& destination) {
    clearDistanceMatrix();
    m_distanceMatrix[destination.x][destination.y] = 0;

    FixedDeque<FloodFillNode> queue(m_MazeWidth*m_MazeHeight);
    queue.push_back( {destination, 0});

    while (!(queue.is_empty())) {
        FloodFillNode node = queue.pop_front();

        for (vec2<int> dir : m_directions) {
            vec2<int> newPos = node.pos + dir;

            // Skip OOB
            if (newPos.x < 0 || newPos.y < 0 || newPos.x >= m_MazeWidth || newPos.y >= m_MazeHeight)
                continue;

            // Skip visited
            if (m_distanceMatrix[newPos.x][newPos.y] != -1)
                continue;

            vec2<int> newPosEx = posToPosEx(newPos);
            vec2<int> wallPos = newPosEx - dir; // step back

            // Can't drive through walls
            if (m_wallMatrix[wallPos.x][wallPos.y] == 1)
                continue;

            m_distanceMatrix[newPos.x][newPos.y] = node.dist + 1;

            queue.push_back({ newPos, node.dist + 1 });
        }

    }
}


void MazeSolver::printWalls() {
#ifdef WINDOWS
    for (int y = 0; y < m_MazeHeightEx; y++) {
        for (int x = 0; x < m_MazeWidthEx; x++) {
            std::cout << (int)(m_wallMatrix[x][y]) << " ";
        }
        std::cout << "\n";
    }
#endif
}

void MazeSolver::printDists() {
#ifdef WINDOWS
    for (int y = 0; y < m_MazeHeight; y++) {
        for (int x = 0; x < m_MazeWidth; x++) {
            std::cout << (int)(m_distanceMatrix[x][y]) << " ";
        }
        std::cout << "\n";
    }
#endif
}
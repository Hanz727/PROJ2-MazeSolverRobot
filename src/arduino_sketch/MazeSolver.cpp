#include "MazeSolver.h"
#include "math.h"

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
            m_wallMatrix[x][y] = 0;
        }
    }
}


vec2<double> MazeSolver::posToCm(const vec2<double>& pos) const {
    return vec2<double>(
        (m_cellWidth + m_wallWidth) * pos.x + (m_wallWidth + m_cellWidth / 2.),
        (m_cellHeight + m_wallWidth) * pos.x + (m_wallWidth + m_cellHeight / 2.)
    );
}

vec2<double> MazeSolver::cmToPos(const vec2<double>& cm) const {
    return vec2<double>(
        (cm.x - m_wallWidth - m_cellWidth / 2.) / (m_cellWidth + m_wallWidth),
        (cm.x - m_wallWidth - m_cellHeight / 2.) / (m_cellHeight + m_wallWidth)
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

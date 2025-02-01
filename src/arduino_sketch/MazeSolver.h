#pragma once
#include "vec2.h"
#include <stdint.h>

// Max maze size is fixed on (65-1)/2 = 32 by 32
using matrix2d = int8_t[65][65];
using directions = int8_t;

enum CompassDir {
    North = (1 << 0),
    South = (1 << 1),
    East  = (1 << 2),
    West  = (1 << 3)
};

struct FloodFillNode {
    vec2<int> pos;
    int dist;
};

class MazeSolver {
private:
    const double m_wallWidth;
    const double m_cellWidth;
    const double m_cellHeight;
private:
    const uint8_t m_MazeWidth;
    const uint8_t m_MazeHeight;
private:
    const uint8_t m_MazeWidthEx;
    const uint8_t m_MazeHeightEx;
private:
    matrix2d m_distanceMatrix{0};
    matrix2d m_wallMatrix{0};
private:
    vec2<int> m_directions[4] = {
        { 0,-1}, // N
        { 0, 1}, // S
        { 1, 0}, // E
        {-1, 0}  // W
    };
private:
    void clearDistanceMatrix();
    void clearWallMatrix();
public:
    const vec2<int> m_startPos;
    const vec2<int> m_endPos;
    vec2<double> m_currPos;
public:
    MazeSolver(const double wallWidth,
        const double cellWidth,
        const double cellHeight,
        const uint8_t mazeWidth,
        const uint8_t mazeHeight,
        const vec2<int> startPos,
        const vec2<int> endPos
    );

    ~MazeSolver();
public:
    vec2<double> posToCm(const vec2<double>& pos) const;
    vec2<double> cmToPos(const vec2<double>& cm) const;

    vec2<int> roundPos(const vec2<double>& pos) const;

public:
    vec2<int> posToPosEx(const vec2<double>& pos) const;
    vec2<double> posExToPos(const vec2<int>& posEx) const;
public:
    void markWall(const vec2<double>& pos, const double distance, CompassDir dir);
    void floodFill(const vec2<int>& destination);
public:
    vec2<int> getDirOffset(CompassDir dir);
    vec2<int> getNextMove();
    directions getPossibleMoves();
public:
    void printWalls();
    void printDists();
};
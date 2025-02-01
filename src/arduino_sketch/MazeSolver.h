#pragma once
#include "vec2.h"
#include <stdint.h>

// Max maze size is fixed on (65-1)/2 = 32 by 32
using matrix2d = int8_t[65][65];

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
    const vec2<int> m_startPos;
    const vec2<int> m_endPos;
private:
    matrix2d m_distanceMatrix{0};
    matrix2d m_wallMatrix{0};
private:
    void clearDistanceMatrix();
    void clearWallMatrix();
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

public:
    vec2<int> posToPosEx(const vec2<double>& pos) const;
    vec2<double> posExToPos(const vec2<int>& posEx) const;

};
#pragma once
#include "vec2.h"
#include <stdint.h>

class MazeSolver {
private:
    const double m_wallWidth;
    const double m_cellWidth;
    const double m_cellHeight;
private:
    const uint8_t m_MazeWidth;
    const uint8_t m_MazeHeight;
private:
    const vec2<int> m_startPos;
    const vec2<int> m_endPos;
private:
    int**  const m_distanceMatrix;
    bool** const m_wallMatrix;
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
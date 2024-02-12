#ifndef MAZEGENERATOR_H
#define MAZEGENERATOR_H

// Function declarations
class Maze {
public:
    Maze(int x, int y);
    void Drill(int ox, int oy);
    void printMaze();
    void renderMaze(float size, float thickness);
    int* hwall;
    int* vwall;
    float pickup[4][2];
private:
    int rows, columns;
    
};

#endif // MAZEGENERATOR_H

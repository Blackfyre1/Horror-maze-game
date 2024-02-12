#ifndef PLAYER_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define PLAYER_H

#include <GL/glut.h>
#include <glfw3.h>
#include <iostream>

void loadTextures();
void drawFloor3D(GLfloat x, GLfloat z, GLfloat size);
void drawCeiling3D(GLfloat x, GLfloat z, GLfloat size);
void drawHorizontal3D(GLfloat x, GLfloat z, GLfloat size,float thickness);
void drawVertical3D(GLfloat x, GLfloat z, GLfloat size, float thickness);
void unloadTextures();
#endif

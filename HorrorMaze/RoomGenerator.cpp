#include <GL/glew.h>
#include "RoomGenerator.h"
#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"


GLuint floorTexture; // Texture ID for the floor texture
unsigned char* image_data;
int w, h, channels;

void loadTextures() {
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &floorTexture);
    

    image_data = stbi_load("Texture/main.jpg", &w, &h, &channels, 0);
    if (image_data) {
        // Enable sRGB conversion for the framebuffer
        glEnable(GL_FRAMEBUFFER_SRGB);
        glGenTextures(1, &floorTexture);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);   
    }
    else {
        std::cerr << "Failed to load floor texture" << std::endl;
    }
}

void unloadTextures() {
    glDeleteTextures(1, &floorTexture);
    stbi_image_free(image_data);
}

void drawFloor3D(GLfloat x, GLfloat z,GLfloat size) {
    GLfloat roomLeft = x * size;
    GLfloat roomRight = (x + 1) * size;
    GLfloat roomBottom = -0.4 * size;
    GLfloat roomTop = 0.5 * size;
    GLfloat roomFront = (z + 1) * size;
    GLfloat roomBack = z * size;

    glBegin(GL_QUADS);

    // Floor
    glTexCoord2f(0.0, 0.0);  glVertex3f(roomLeft, roomBottom, roomFront);
    glTexCoord2f(1.0, 0.0);  glVertex3f(roomRight, roomBottom, roomFront);
    glTexCoord2f(1.0, 1.0);  glVertex3f(roomRight, roomBottom, roomBack);
    glTexCoord2f(0.0, 1.0);  glVertex3f(roomLeft, roomBottom, roomBack);

    glEnd();
}

void drawCeiling3D(GLfloat x, GLfloat z, GLfloat size) {
    GLfloat roomLeft = x * size;
    GLfloat roomRight = (x + 1) * size;
    GLfloat roomBottom = -0.4 * size;
    GLfloat roomTop = 0.5 * size;
    GLfloat roomFront = (z + 1) * size;
    GLfloat roomBack = z * size;
    glBegin(GL_QUADS);

    // Ceiling
    glTexCoord2f(0.0, 0.0);  glVertex3f(roomLeft, roomTop, roomFront);
    glTexCoord2f(1.0, 0.0);  glVertex3f(roomRight, roomTop, roomFront);
    glTexCoord2f(1.0, 1.0);  glVertex3f(roomRight, roomTop, roomBack);
    glTexCoord2f(0.0, 1.0);  glVertex3f(roomLeft, roomTop, roomBack);

    glEnd();
}

void drawHorizontal3D(GLfloat x, GLfloat z, GLfloat size,GLfloat thickness) {
    GLfloat roomLeft = x * size;
    GLfloat roomRight = (x + 1) * size;
    GLfloat roomBottom = -0.4 * size;
    GLfloat roomTop = 0.5 * size;
    GLfloat roomFront = (z + 1) * size;
    GLfloat roomBack = z * size;

    glBegin(GL_QUADS);

    // Front wall
    glTexCoord2f(0.0, 0.0);  glVertex3f(roomLeft, roomBottom, roomBack *(1-thickness));
    glTexCoord2f(1.0, 0.0);  glVertex3f(roomRight, roomBottom, roomBack * (1 - thickness));
    glTexCoord2f(1.0, 1.0);  glVertex3f(roomRight, roomTop, roomBack * (1 - thickness));
    glTexCoord2f(0.0, 1.0);  glVertex3f(roomLeft, roomTop, roomBack * (1 - thickness));

    glTexCoord2f(0.0, 0.0);  glVertex3f(roomLeft, roomBottom, roomBack * (1 + thickness));
    glTexCoord2f(1.0, 0.0);  glVertex3f(roomRight, roomBottom, roomBack * (1 + thickness));
    glTexCoord2f(1.0, 1.0);  glVertex3f(roomRight, roomTop, roomBack * (1 + thickness));
    glTexCoord2f(0.0, 1.0);  glVertex3f(roomLeft, roomTop, roomBack * (1 + thickness));

    glTexCoord2f(0.0, 0.0);  glVertex3f(roomLeft, roomBottom, roomBack * (1 - thickness));
    glTexCoord2f(1.0, 0.0);  glVertex3f(roomLeft, roomBottom, roomBack * (1 + thickness));
    glTexCoord2f(1.0, 1.0);  glVertex3f(roomLeft, roomTop, roomBack * (1 + thickness));
    glTexCoord2f(0.0, 1.0);  glVertex3f(roomLeft, roomTop, roomBack * (1 - thickness));

    glTexCoord2f(0.0, 0.0);  glVertex3f(roomRight, roomBottom, roomBack * (1 - thickness));
    glTexCoord2f(1.0, 0.0);  glVertex3f(roomRight, roomBottom, roomBack * (1 + thickness));
    glTexCoord2f(1.0, 1.0);  glVertex3f(roomRight, roomTop, roomBack * (1 + thickness));
    glTexCoord2f(0.0, 1.0);  glVertex3f(roomRight, roomTop, roomBack * (1 - thickness));

    glEnd();
}

void drawVertical3D(GLfloat x, GLfloat z, GLfloat size, GLfloat thickness) {
    GLfloat roomLeft = x * size;
    GLfloat roomRight = (x + 1) * size;
    GLfloat roomBottom = -0.4 * size;
    GLfloat roomTop = 0.5*size;
    GLfloat roomFront = (z + 1) * size;
    GLfloat roomBack = z * size;

    glBegin(GL_QUADS);

    // Right wall
    glTexCoord2f(0.0, 0.0);  glVertex3f(roomLeft * (1 - thickness), roomBottom, roomFront);
    glTexCoord2f(1.0, 0.0);  glVertex3f(roomLeft * (1 - thickness), roomBottom, roomBack);
    glTexCoord2f(1.0, 1.0);  glVertex3f(roomLeft * (1 - thickness), roomTop, roomBack);
    glTexCoord2f(0.0, 1.0);  glVertex3f(roomLeft * (1 - thickness), roomTop, roomFront);

    glTexCoord2f(0.0, 0.0);  glVertex3f(roomLeft * (1 + thickness), roomBottom, roomFront);
    glTexCoord2f(1.0, 0.0);  glVertex3f(roomLeft * (1 + thickness), roomBottom, roomBack);
    glTexCoord2f(1.0, 1.0);  glVertex3f(roomLeft * (1 + thickness), roomTop, roomBack);
    glTexCoord2f(0.0, 1.0);  glVertex3f(roomLeft * (1 + thickness), roomTop, roomFront);

    glTexCoord2f(0.0, 0.0);  glVertex3f(roomLeft * (1 - thickness), roomBottom, roomFront);
    glTexCoord2f(1.0, 0.0);  glVertex3f(roomLeft * (1 + thickness), roomBottom, roomFront);
    glTexCoord2f(1.0, 1.0);  glVertex3f(roomLeft * (1 + thickness), roomTop, roomFront);
    glTexCoord2f(0.0, 1.0);  glVertex3f(roomLeft * (1 - thickness), roomTop, roomFront);

    glTexCoord2f(0.0, 0.0);  glVertex3f(roomLeft * (1 - thickness), roomBottom, roomBack);
    glTexCoord2f(1.0, 0.0);  glVertex3f(roomLeft * (1 + thickness), roomBottom, roomBack);
    glTexCoord2f(1.0, 1.0);  glVertex3f(roomLeft * (1 + thickness), roomTop, roomBack);
    glTexCoord2f(0.0, 1.0);  glVertex3f(roomLeft * (1 - thickness), roomTop, roomBack);

    glEnd();
}
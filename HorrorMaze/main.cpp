#include <GL/glut.h>
#include <math.h>
#include<iostream>
#include<string>
#include "RoomGenerator.h"
#include "MazeGenerator.h"
#include "enemy.h"
#include <AL/al.h>
#include <AL/alc.h>
#include<vector>
#include <thread>

#define FPS 60
#define TO_RADIANS 3.14/180.0
#define M_PI 3.14159265358979323846

//Maze variables
int cellSize = 30;
float wallThickness = 0.025;
float thickness = 0.060;
int rows = 10, columns = 10;
int playerCellX = 0, playerCellZ = 0; //initial starting cell
Maze *m = new Maze(rows,columns);


//enemy parameters
float enemyHitbox = 5;
float enemyDetectionRange = 3;
Enemy *enemyList[5];
int enemies = 0;

//game end conditions
bool loss = 0;
int picked = 0;


//width and height of the window ( Aspect ration 16:9 )
const int width = 16 * 50;
const int height = 9 * 50;

//camera and moving variables
float camX = 10.0, camY = 5,camZ = 10.0;
float yOffset = 0;
float pitch = 0.0, yaw = -180.0;
float bobbingAmplitude = 0.025f;float bobbingFrequency = 5.0f;float currtime = 0.0f;
float idleBobbingAmplitude = 0.025f;float idleBobbingFrequency = 5.0f;
float walkBobbingAmplitude = 0.04f;float walkBobbingFrequency = 30.0f;
float sprintBobbingAmplitude = 0.06f;float sprintBobbingFrequency = 60.0f;
float speed = 0.0f;float walkSpeed = 1.0f;float runSpeed = 3.0f;
float maxStamina = 100.0f;
float currentStamina = maxStamina;
float staminaDepletionRate = 0.5f;
float staminaRecoveryRate = 0.25f;
bool chased = false;
struct Motion
{
    bool Forward, Backward, Left, Right,Sprint;
};
Motion motion = { false,false,false,false,false };

//pickup item class

class KeyPickup {
public:
    void draw(GLfloat x, GLfloat y, GLfloat z, GLfloat size, GLfloat r, GLfloat g, GLfloat b) {
        // Draw the bottom pyramid
        glEnable(GL_COLOR_MATERIAL);
        glColor3f(r, g, b);
        glPushMatrix();
        glTranslatef(x, y + size, z);
        glBegin(GL_TRIANGLES);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-1.0f * size, -1.0f * size, 1.0f * size);
        glVertex3f(1.0f * size, -1.0f * size, 1.0f * size);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(1.0f * size, -1.0f * size, 1.0f * size);
        glVertex3f(0.0f, -1.0f * size, -1.0f * size);
        glVertex3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, -1.0f * size, -1.0f * size);
        glVertex3f(-1.0f * size, -1.0f * size, 1.0f * size);
        glVertex3f(-1.0f * size, -1.0f * size, 1.0f * size);
        glVertex3f(1.0f * size, -1.0f * size, 1.0f * size);
        glVertex3f(0.0f, -1.0f * size, -1.0f * size);
        glEnd();
        glPopMatrix();

        //top pyramid
        glPushMatrix();
        glTranslatef(x, y - size, z);
        glBegin(GL_TRIANGLES);
        glVertex3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-1.0f * size, 1.0f * size, 1.0f * size);
        glVertex3f(1.0f * size, 1.0f * size, 1.0f * size);
        glVertex3f(0.0f, -1.0f, 0.0f);
        glVertex3f(1.0f * size, 1.0f * size, 1.0f * size);
        glVertex3f(0.0f, 1.0f * size, -1.0f * size);
        glVertex3f(0.0f, -1.0f, 0.0f);
        glVertex3f(0.0f, 1.0f * size, -1.0f * size);
        glVertex3f(-1.0f * size, 1.0f * size, 1.0f * size);
        glVertex3f(-1.0f * size, 1.0f * size, 1.0f * size);
        glVertex3f(1.0f * size, 1.0f * size, 1.0f * size);
        glVertex3f(0.0f, 1.0f * size, -1.0f * size);
        glEnd();
        glPopMatrix();
    }
};

//Audio variables
ALCdevice* device = nullptr;
ALCcontext* context = nullptr;
constexpr int NUM_SOURCES = 5;
ALuint audioSources[NUM_SOURCES];

//Audio functions.

ALuint loadAudio(const char* filename) {
    ALuint buffer;
    alGenBuffers(1, &buffer);
    FILE* file;
    if (fopen_s(&file,filename, "rb") != 0) {
        std::cout << "Failed to open file" << std::endl;
        return 0;
    }
    else {
        std::cout << filename<<"\n";
    }
    std::vector<char> bufferData;
    const int headerSize = 44;
    bufferData.resize(headerSize);
    fread(bufferData.data(), sizeof(char), headerSize, file);

    fseek(file, 0, SEEK_END);
    size_t dataSize = ftell(file) - headerSize;
    bufferData.resize(dataSize + headerSize);

    fseek(file, headerSize, SEEK_SET);
    fread(bufferData.data() + headerSize, sizeof(char), dataSize, file);

    alBufferData(buffer, AL_FORMAT_MONO16, bufferData.data(), bufferData.size(), 44100);

    fclose(file);
    return buffer;
}

// Function to play the loaded audio
void playIdleAudio(int index,ALuint buffer, float volume) {
    ALuint source = audioSources[index];
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_GAIN, volume);

    alSourcePlay(source);

    while (true) {
        ALint sourceState;
        alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
        if (chased && sourceState == AL_PLAYING)
        {
            alSourcePause(source);
        }
        else if (!chased && sourceState != AL_PLAYING)
        {
            alSourcePlay(source);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
}

void playChaseAudio(int index, ALuint buffer, float volume) {
    ALuint source = audioSources[index];
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_GAIN, volume);
    while (true) {
        ALint sourceState;
        alGetSourcei(source, AL_SOURCE_STATE, &sourceState);

        if (chased && sourceState != AL_PLAYING) {
            alSourcePlay(source);
        }
        else if (!chased && sourceState == AL_PLAYING) {
            alSourceStop(source);
        }
    }
}

void playAudioOnce(int index, ALuint buffer, float volume) {
    ALuint source = audioSources[index];
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_GAIN, volume);
    alSourcePlay(source);
    ALint sourceState;
    alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
    do {
        alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
    } while (sourceState == AL_PLAYING);
}

void playWalkAudio(int index, ALuint buffer, float volume) {
    ALuint source = audioSources[index];
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_GAIN, volume);

    while (true) {
        bool check = ((motion.Forward||motion.Backward||motion.Left||motion.Right) && !motion.Sprint);
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust delay if needed
        ALint sourceState;
        alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
        // Check if the source has stopped playing (audio finished)
        if (sourceState != AL_PLAYING && check) {
            alSourceRewind(source); // Reposition cursor to the beginning
            alSourcePlay(source);   // Restart audio playback
        }
        else if (sourceState == AL_PLAYING && !check)
        {
            alSourceStop(source);
        }
    }
}

void playRunAudio(int index, ALuint buffer, float volume) {
    ALuint source = audioSources[index];
    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_GAIN, volume);

    while (true) {
        bool check = ((motion.Forward || motion.Backward || motion.Left || motion.Right) && motion.Sprint);
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Adjust delay if needed
        ALint sourceState;
        alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
        // Check if the source has stopped playing (audio finished)
        if (sourceState != AL_PLAYING && check) {
            alSourceRewind(source); // Reposition cursor to the beginning
            alSourcePlay(source);   // Restart audio playback
        }
        else if (sourceState == AL_PLAYING && !check)
        {
            alSourceStop(source);
        }
    }
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'W':
    case 'w':
        motion.Forward = true;
        break;
    case 'A':
    case 'a':
        motion.Left = true;
        break;
    case 'S':
    case 's':
        motion.Backward = true;
        break;
    case 'D':
    case 'd':
        motion.Right = true;
        break;
    }
}

void keyboard_up(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'W':
    case 'w':
        motion.Forward = false;
        break;
    case 'A':
    case 'a':
        motion.Left = false;
        break;
    case 'S':
    case 's':
        motion.Backward = false;
        break;
    case 'D':
    case 'd':
        motion.Right = false;
        break;
    }   
}

void specialKeyboard(int key, int x, int y) {
    switch (key) {
    case 112:
        motion.Sprint = true;
        break;
    }
}

void specialKeyboardUp(int key, int x, int y) {
    switch (key) {
    case 112:
        motion.Sprint = false;
        break;
    }
}



void passive_motion(int x, int y)
{
    int dev_x, dev_y;
    dev_x = (width / 2) - x;
    dev_y = (height / 2) - y;

    yaw += (float)dev_x/10.0;
    pitch += (float)dev_y/10.0;

    // Keep pitch within desired bounds
    if (pitch > 90.0f)
        pitch = 90.0f;
    if (pitch < -90.0f)
        pitch = -90.0f;

    // Limit yaw to avoid gimbal lock
    if (yaw >= 360.0f)
        yaw -= 360.0f;
    if (yaw <= 0.0f)
        yaw += 360.0f;

    glutWarpPointer(width / 2, height / 2);
}


void camera()
{
    if (motion.Sprint && currentStamina > 0.0f)
    {
        currentStamina -= staminaDepletionRate;
        speed = runSpeed;
        bobbingAmplitude = sprintBobbingAmplitude;
        bobbingFrequency = sprintBobbingFrequency;
    }
    else if (motion.Sprint)
    {
        motion.Sprint = false;  // Sprint is disabled when stamina is depleted
        if (currentStamina < maxStamina)
        {
            currentStamina += staminaRecoveryRate;
        }
    }
    else if (motion.Forward || motion.Backward || motion.Left || motion.Right)
    {
        speed = walkSpeed;
        bobbingAmplitude = walkBobbingAmplitude;
        bobbingFrequency = walkBobbingFrequency;
        if (currentStamina < maxStamina)
        {
            currentStamina += staminaRecoveryRate;
        }
    }
    else
    {
        speed = walkSpeed;
        bobbingAmplitude = idleBobbingAmplitude;
        bobbingFrequency = idleBobbingFrequency;
        if (currentStamina < maxStamina)
        {
            currentStamina += staminaRecoveryRate;
        }
    }

    yOffset = bobbingAmplitude * sin(bobbingFrequency * currtime);
    if (motion.Forward)
    {
        camX += speed * cos((yaw + 90) * TO_RADIANS) / 5.0;
        camZ -= speed * sin((yaw + 90) * TO_RADIANS) / 5.0;
    }
    if (motion.Backward)
    {
        camX += speed * cos((yaw + 90 + 180) * TO_RADIANS) / 5.0;
        camZ -= speed * sin((yaw + 90 + 180) * TO_RADIANS) / 5.0;
    }
    if (motion.Left)
    {
        camX += speed * cos((yaw + 90 + 90) * TO_RADIANS) / 5.0;
        camZ -= speed * sin((yaw + 90 + 90) * TO_RADIANS) / 5.0;
    }
    if (motion.Right)
    {
        camX += speed * cos((yaw + 90 - 90) * TO_RADIANS) / 5.0;
        camZ -= speed * sin((yaw + 90 - 90) * TO_RADIANS) / 5.0;
    }
    if (camX - playerCellX * cellSize >= cellSize * (1 - thickness) && (playerCellX == rows - 1 || m->vwall[playerCellZ * (columns+1) + playerCellX + 1] == 1)) {
        camX = (playerCellX + 1 - thickness) * cellSize;
    }
    else if(camX - playerCellX * cellSize >= cellSize * (1 - thickness) && (playerCellX != rows - 1 && m->vwall[playerCellZ * (columns+1) + playerCellX + 1] == 0)) {
        playerCellX ++;
    }
    else if (camX - playerCellX * cellSize <= cellSize * thickness && (playerCellX == 0 || m->vwall[playerCellZ * (columns+1) + playerCellX] == 1)) {
        camX = (playerCellX + thickness) * cellSize;
    }
    else if (camX - playerCellX * cellSize <= cellSize * thickness && (playerCellX != 0 && m->vwall[playerCellZ * (columns + 1) + playerCellX] == 0)) {
        playerCellX--;
    }

    if (camZ - playerCellZ * cellSize >= cellSize * (1 - thickness) && (playerCellZ == columns - 1 || m->hwall[playerCellZ * (columns) + playerCellX + columns] == 1)) {
        camZ = (playerCellZ + 1 - thickness) * cellSize;
    }
    else if (camZ - playerCellZ * cellSize >= cellSize * (1 - thickness) && (playerCellZ != columns - 1 && m->hwall[playerCellZ * (columns) + playerCellX + columns] == 0)) {
        playerCellZ++;
    }
    else if (camZ - playerCellZ * cellSize <= cellSize * thickness && (playerCellZ == 0 || m->hwall[playerCellZ * columns + playerCellX] == 1)) {
        camZ = (playerCellZ + thickness) * cellSize;
    }
    else if (camZ - playerCellZ * cellSize <= cellSize * thickness && (playerCellZ != 0 && m->hwall[playerCellZ * columns + playerCellX] == 0)) {
        playerCellZ--;
    }
    /*limit the values of pitch
      between -60 and 70
    */
    if (pitch >= 70)
        pitch = 70;
    if (pitch <= -60)
        pitch = -60;
    camY = yOffset;
    glRotatef(-pitch, 1.0, 0.0, 0.0); // Along X axis
    glRotatef(-yaw, 0.0, 1.0, 0.0);    //Along Y axis
    glTranslatef(-camX, -camY, -camZ);
}

void drawText(const char* text, int x, int y) {
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the text
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, width, height, 0.0, -1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    //glPushMatrix();        ----Not sure if I need this
    glLoadIdentity();
    glDisable(GL_CULL_FACE);
    //glDisable(GL_LIGHTING);
    glClear(GL_DEPTH_BUFFER_BIT);
    glRasterPos2i(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, *text++);
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void drawStaminaBar() {
    // Calculate the length of the stamina bar based on currentStamina and maxStamina
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0, width, height, 0.0, -1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
    float barLength = (currentStamina / maxStamina) * (width / 4); // Adjust width as needed
    float barHeight = 20.0f; // Adjust height as needed
    float barPosX = 20.0f; // Adjust position as needed
    float barPosY = height - 40.0f; // Adjust position as needed
    glDisable(GL_CULL_FACE);
    //glDisable(GL_LIGHTING);
    glClear(GL_DEPTH_BUFFER_BIT);
    glRasterPos2i(barPosX, barPosY);
    // Draw the outline of the stamina bar
    glColor3f(1.0f, 1.0f, 1.0f); // White color
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(barPosX, barPosY);
    glVertex2f(barPosX + width / 4, barPosY);
    glVertex2f(barPosX + width / 4, barPosY + barHeight);
    glVertex2f(barPosX, barPosY + barHeight);
    glEnd();

    // Draw the filled portion of the stamina bar
    glColor3f(0.0f, 0.2f, 0.0f); // Green color
    glBegin(GL_QUADS);
    glVertex2f(barPosX, barPosY);
    glVertex2f(barPosX + barLength, barPosY);
    glVertex2f(barPosX + barLength, barPosY + barHeight);
    glVertex2f(barPosX, barPosY + barHeight);
    glEnd();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glColor3f(0.5f, 0.5f, 0.5f);
}

void drawCylinder(float radius, float height, int slices) {
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, height); // Center of top face
    for (int i = 0; i <= slices; ++i) {
        float angle = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(slices);
        glVertex3f(radius * cos(angle), radius * sin(angle), height); // Vertices of top face
    }
    glEnd();

    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0.0f, 0.0f, 0.0f); // Center of bottom face
    for (int i = 0; i <= slices; ++i) {
        float angle = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(slices);
        glVertex3f(radius * cos(angle), radius * sin(angle), 0.0f); // Vertices of bottom face
    }
    glEnd();

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float angle = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(slices);
        glVertex3f(radius * cos(angle), radius * sin(angle), 0.0f); // Bottom vertices
        glVertex3f(radius * cos(angle), radius * sin(angle), height); // Top vertices
    }
    glEnd();
}

void drawGhost(float x, float y, float z) {
    const int slices = 200;

    // Body (create a wavy pattern using sine waves)
    glTranslatef(x, 0, z);
    glScalef(2, 2, 2);
    // Body (create a wavy pattern using sine waves)
    glColor3f(0.25f, 0.0f, 0.0f); // Light gray color
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float angle = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(slices);
        float x = cos(angle);
        float y = sin(angle);
        float z = sin(12.0f * angle); // Increased the frequency of waves
        
        // Adjusted scaling for a larger figure at the bottom and a slimmer figure at the top
        float scaleY = 3.0f - 2.5f * (static_cast<float>(i) / static_cast<float>(slices));
        glVertex3f(1.0f * x, scaleY * y, 1.0f * z);
        glVertex3f(0.8f * x, scaleY * y, 0.8f * z); // Create the wavy edge
    }
    glEnd();

    // Connect the edges with faces
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= slices; ++i) {
        float angle = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(slices);
        float x = cos(angle);
        float y = sin(angle);
        float z = sin(12.0f * angle); // Same sine wave function
        glVertex3f(1.0f * x, 3.0f * y, 1.0f * z); // Upper wavy edge
        glVertex3f(1.0f * x, -3.0f * y, 1.0f * z); // Lower wavy edge
    }
    glEnd();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    camera();
    m->renderMaze(cellSize,wallThickness);
    KeyPickup key[4];
    key[0].draw(m->pickup[0][0]* cellSize, -0.5, m->pickup[0][1] * cellSize, 2, 0.5, 0, 0.5);
    key[1].draw(m->pickup[1][0] * cellSize, -0.5, m->pickup[1][1] * cellSize, 2, 1.0, 0, 0);
    key[2].draw(m->pickup[2][0] * cellSize, -0.5, m->pickup[2][1] * cellSize, 2, 0, 1.0, 0);
    key[3].draw(m->pickup[3][0] * cellSize, -0.5, m->pickup[3][1] * cellSize, 2, 0, 0, 1.0);
    for (int i = 0;i < enemies;i++)
    {
        drawGhost(enemyList[i]->enemyX, 0, enemyList[i]->enemyZ);
    }
    if (picked == 4)
    {
        drawText("You Win!!", width/2, height/2);
    }
    else if (loss)
    {
        drawText("You Died", width * 0.4, height * 0.5);
    }
    else 
    {
        std::string s = std::to_string(picked) + "/4";
        drawText(s.c_str(), width * 0.1, height * 0.9);
        drawStaminaBar();
    }
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 16.0 / 9.0, 1, 100);
    glMatrixMode(GL_MODELVIEW);

}

void timer(int)
{
    glutPostRedisplay();
    glutWarpPointer(width / 2, height / 2);
    glutTimerFunc(1000 / FPS, timer, 0);
    currtime += 0.01;
}



void init()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glutWarpPointer(width / 2, height / 2);
    glutSetCursor(GLUT_CURSOR_NONE);
    glDepthFunc(GL_LEQUAL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void initIdleAudio()
{
    ALuint audioBuffer = loadAudio("Audio/Idle.wav");
    playIdleAudio(0,audioBuffer, 0.5);
}

void initWalkAudio()
{
    ALuint audioBuffer = loadAudio("Audio/Walk.wav");
    playWalkAudio(1, audioBuffer, 0.4);
}

void initRunAudio()
{
    ALuint audioBuffer = loadAudio("Audio/Run.wav");
    playRunAudio(2, audioBuffer, 0.6);
}

void initChaseAudio()
{
    ALuint audioBuffer = loadAudio("Audio/Chase.wav");
    playChaseAudio(3,audioBuffer, 0.4);
}

void checkpickup(float locations[4][2], const char* path) {
    float distance;
    ALuint audioBuffer = loadAudio(path);
    while (true)
    {
        for (int i = 0;i < 4;i++)
        {
            distance = pow((locations[i][0] - camX), 2);
            distance += pow((locations[i][1] - camZ), 2);
            distance = sqrt(distance);
            if (distance < 5)
            {
                picked++;
                m->pickup[i][0] = -1000;
                m->pickup[i][1] = -1000;
                locations[i][0] = -1000;
                locations[i][1] = -1000;
                playAudioOnce(4,audioBuffer,0.6);
            }
        }
    }
}

void pickup() {
    float locations[4][2];
    for (int i = 0;i < 4;i++)
    {
        locations[i][0] = m->pickup[i][0] * cellSize;
        locations[i][1] = m->pickup[i][1] * cellSize;
    }
    checkpickup(locations, "Audio/picked.wav");
}

void waitFor60Seconds() {
    std::this_thread::sleep_for(std::chrono::seconds(60));
}

void spawnEnemyRandomly(int cellSize) {
    srand(static_cast<unsigned int>(time(nullptr)));

    for (int i = 0; i < 5; ++i) {
        Enemy *e = new Enemy (0.0,0.0,0.35);
        float posX,posZ;
        do {
            int randomX = rand() % rows; // Random X position between 0 and 99
            int randomZ = rand() % columns; // Random Z position between 0 and 99
            posX = ((2 * randomX + 1) * cellSize) / 2.0f;
            posZ = ((2 * randomZ + 1) * cellSize) / 2.0f;
        } while (e->checkCollision(camX, camZ, enemyHitbox));
        e->spawnEnemy(posX, posZ);
        enemyList[i] = e;
        enemies++;
        waitFor60Seconds();
    }
}

void enemyInit() {
    spawnEnemyRandomly(cellSize);
}

void enemyUpdate() {
    while (true)
    {
        bool x = false;
        for (int i = 0;i < enemies;i++)
        {
            Enemy* e = enemyList[i];
            if (e->checkCollision(camX, camZ, enemyHitbox))
            {
                loss = 1;
            }
            else if (e->checkRange(camX, camZ, cellSize * enemyDetectionRange))
            {
                e->chasePlayer(camX, camZ);
                x = true;
            }
        }
        chased = x;
        if (loss)
        {
            break;
        }
        std::chrono::duration<int, std::ratio<1, 60>> sixtiethOfSecond(1);
        std::this_thread::sleep_for(sixtiethOfSecond);
    }
}

void initAudioDevice() {
    device = alcOpenDevice(nullptr);
    if (!device) {
        std::cout << "Failed to open audio device" << std::endl;
        return;
    }

    context = alcCreateContext(device, nullptr);
    if (!context) {
        std::cout << "Failed to create audio context" << std::endl;
        alcCloseDevice(device);
        return;
    }

    alcMakeContextCurrent(context);
    alGenSources(NUM_SOURCES, audioSources);

    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
        std::cout << "Error generating sources: " << error << std::endl;
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("Purgatory");
    init();
    initAudioDevice();

    std::thread audioIdleThread(initIdleAudio);
    std::thread audioWalkThread(initWalkAudio);
    std::thread audioRunThread(initRunAudio);
    std::thread chase(initChaseAudio);
    std::thread pick(pickup);
    std::thread enemy(enemyInit);
    std::thread update(enemyUpdate);

    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyboard_up);
    glutSpecialFunc(specialKeyboard);
    glutSpecialUpFunc(specialKeyboardUp);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glutPassiveMotionFunc(passive_motion);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}

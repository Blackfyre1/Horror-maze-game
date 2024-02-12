#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include<thread>
#include<chrono>
#include"enemy.h"
#include<iostream>


Enemy::Enemy(float x, float z, float speed) : enemyX(x), enemyZ(z), enemySpeed(speed) {}

void Enemy::chasePlayer(float playerX, float playerZ) {
    float directionX = playerX - enemyX;
    float directionZ = playerZ - enemyZ;

    // Normalize direction
    float length = sqrt(directionX * directionX + directionZ * directionZ);
    directionX /= length;
    directionZ /= length;
    if (length < enemySpeed)
    {
        enemyX += directionX * length;
        enemyZ += directionZ * length;
    }
    else
    {
        // Update enemy position to move towards the player
        enemyX += directionX * enemySpeed;
        enemyZ += directionZ * enemySpeed;
    }
}
bool Enemy::checkRange(float playerX, float playerZ, float detectionRange) {
    // Calculate distance between player and enemy
    float distance = sqrt((playerX - enemyX) * (playerX - enemyX) + (playerZ - enemyZ) * (playerZ - enemyZ));
    //std::cout << distance <<" "<<detectionRange<< "\n";
    return distance < detectionRange;
}

bool Enemy::checkCollision(float playerX, float playerZ, float collisionThreshold) {    // Calculate distance between player and enemy
    float distance = sqrt((playerX - enemyX) * (playerX - enemyX) + (playerZ - enemyZ) * (playerZ - enemyZ));

    // Collision occurs if the distance between player and enemy is less than a certain threshold
    return distance < collisionThreshold;
}

void Enemy::spawnEnemy(float x, float z) {
    enemyX = x;
    enemyZ = z;
}

void Enemy::printPos() {
    //std::cout << enemyX << " " << enemyZ << "\n";
}
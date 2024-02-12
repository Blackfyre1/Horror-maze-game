#ifndef ENEMY_H
#define ENEMY_H

class Enemy {
private:
    float enemySpeed = 1.2f;

public:
    Enemy(float x, float z, float speed);
    Enemy();
    void chasePlayer(float playerX, float playerZ);

    bool checkRange(float playerX, float playerZ, float detectionRange);

    bool checkCollision(float playerX, float playerZ, float collisionThreshold);

    void spawnEnemy(float x, float z);

    void printPos();

    float enemyX; // Enemy position
    float enemyZ;

};

#endif // ENEMY_H

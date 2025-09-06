#include "raylib.h"
#include <string>
#include <thread>
#include <random>
#include <vector>
#include <atomic>
#include <chrono>

#define CUSTOM_BLUE CLITERAL(Color){22, 83, 247, 255}

std::atomic<bool> timerTick(false);
std::atomic<bool> running(true);
bool collectorsHaveProtection = false;
std::atomic<bool> collectorsHaveProtectionAtomic(collectorsHaveProtection);
bool collectorsHaveSpeedBoost = false;
std::atomic<bool> collectorsHaveSpeedBoostAtomic(collectorsHaveSpeedBoost);

void Timer() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        timerTick = true;
    }
}

void ProtectionTimer() {
    while (running) {
        if (collectorsHaveProtectionAtomic) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            collectorsHaveProtectionAtomic = false;
            collectorsHaveProtection = false;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

void SpeedBoostTimer() {
    while (running) {
        if (collectorsHaveSpeedBoostAtomic) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            collectorsHaveSpeedBoostAtomic = false;
            collectorsHaveSpeedBoost = false;
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

constexpr int WINDOW_WIDTH = 1000;
constexpr int WINDOW_HEIGHT = 800;
int score = 0;
int velocity = 6;
class Collector {
public: 
    int posX = (GetScreenWidth() - 150) / 2;
    int posY = GetScreenHeight() - 70;
    int collector_velocity = 6;
    static const int width = 150;
    static const int height = 30;
    Collector() {
        
    }
    void paint() {
        DrawRectangle(posX, posY, width, height, WHITE);
    }
    void moveLeft() {
        if (posX > 0) {
            if (collectorsHaveSpeedBoost) {
                posX -= collector_velocity * 2;
            } else {
                posX -= collector_velocity;
            }
        }
    }
    void moveRight() {
        if (posX < GetScreenWidth() - width) {
            if (collectorsHaveSpeedBoost) {
                posX += collector_velocity * 2;
            } else {
                posX += collector_velocity;
            }
        }
    }
    Rectangle getRectangle() {
        Rectangle rect = {(float)posX, (float)posY, (float)width, (float)height};
        return rect;
    }
};

class Turret {
public: 
    int posX;
    int posY;
    int width;
    int height;
    bool shouldBeDestroyed = false;
    Collector collector;
    Turret(Collector collector) {
        this->collector = collector;
        this->posX = (collector.posX + (collector.width / 2));
        this->posY = collector.posY - 10;
        this->width = 10;
        this->height = 25;
    }
    void Update() {
        posY -= velocity;
        if (posY < -10) {
            shouldBeDestroyed = true;
        }
    }
    Rectangle getRectangle() {
        Rectangle rect = {(float)posX, (float)posY, (float)width, (float)height};
        return rect;
    }
};

class SpawnTurrets {
public: 
    std::vector<Turret> turrets = {};
    void addToTurrets(Collector collector) {
        turrets.push_back(Turret(collector));
        if (score > 50) {
            Turret turret1 = Turret(collector);
            Turret turret2 = Turret(collector);
            Turret turret3 = Turret(collector);
            Turret turret4 = Turret(collector);
            turret1.posX -= 70;
            turret2.posX -= 35;
            turret3.posX += 35;
            turret4.posX += 70;
            turrets.push_back(turret1);
            turrets.push_back(turret2);
            turrets.push_back(turret3);
            turrets.push_back(turret4);
        } else if (score > 30) {
            Turret turret1 = Turret(collector);
            Turret turret3 = Turret(collector);
            turret1.posX -= 35;
            turret3.posX += 35;
            turrets.push_back(turret1);
            turrets.push_back(turret3);
        }
    }
    
    void drawTurret() {
        for (auto &turret: turrets) {
            DrawRectangle(turret.posX, turret.posY, turret.width, turret.height, GREEN);
        }
    }
    void checkForRemoveableTurrets() {
        for (int i = 0; i < turrets.size(); i++) {
            if (turrets.at(i).shouldBeDestroyed) {
                turrets.erase(turrets.begin() + i);
                i--;
            }
        }
    }
    void Update() {
        for (auto &turret: turrets) {
            turret.Update();
        }
    }
    std::vector<Turret> getTurrets() {
        return turrets;
    }
};

class Collectible {
public: 
    int radius = 25;
    int centerX, centerY;
    bool isGoodBall;
    bool shouldBeDestroyed = false;
    
    Collectible() {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<int> distX(50, GetScreenWidth() - 50);
        static std::bernoulli_distribution distBool(0.5);
        centerX = distX(rng);
        centerY = 100;
        isGoodBall = distBool(rng);
    }
    void paint() {
        DrawCircle(centerX, centerY, radius, isGoodBall ? GREEN : RED);
    }
    void Update() {
        if (centerY - radius < WINDOW_HEIGHT) {
            centerY += velocity;
        }
        if (centerY - radius >= WINDOW_HEIGHT) {
            shouldBeDestroyed = true;
        }
    }
    int getRadius() {
        return (float)radius;
    }
    Vector2 getCenter() {
        Vector2 center = {(float)centerX, (float)centerY};
        return center;
    }
};

class SpawnCollectibles {
public: 
    int spawn_rate;
    std::vector<Collectible> collectibles = {};
    SpawnCollectibles() {
        if (score > 50) {
            spawn_rate = 10;
        } else if (score > 30) {
            spawn_rate = 5;
        } else if (score > 15) {
            spawn_rate = 3;
        } else if (score > 5) {
            spawn_rate = 2;
        } else if (score >= 0) {
            spawn_rate = 1;
        }
    }
    void SetSpawnRate() {
        if (score > 50) {
            spawn_rate = 10;
        } else if (score > 30) {
            spawn_rate = 5;
        } else if (score > 15) {
            spawn_rate = 3;
        } else if (score > 5) {
            spawn_rate = 2;
        } else if (score >= 0) {
            spawn_rate = 1;
        }
    }
    void CheckForRemoval() {
        for (int i = 0; i < collectibles.size(); i++) {
            if (collectibles.at(i).shouldBeDestroyed) {
                collectibles.erase(collectibles.begin() + i);
                i--;
            }
        }
    }
    void addToCollectibles() {
        for (int i = 0; i < spawn_rate; i++) {
            collectibles.push_back(Collectible());
        }
    }
    void spawn() {
        for (auto &collectible: collectibles) {
            collectible.Update();
            collectible.paint();
        }
    }
    void CheckForCollision(Collector collector, SpawnTurrets spawn_turrets) {
        std::vector<Turret> turrets = spawn_turrets.getTurrets();
        static std::bernoulli_distribution RandomBool(0.5);
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<int> random_25_percent_chance(1, 4);
        for (auto &collectible: collectibles) {
            if (CheckCollisionCircleRec(collectible.getCenter(), collectible.getRadius(), collector.getRectangle())) {
                if (collectible.isGoodBall) {
                    score++;
                } else if (score != 0 && !collectible.isGoodBall && !collectorsHaveProtection) {
                    score--;
                }
                collectible.shouldBeDestroyed = true;
            }
            for (auto &turret: turrets) {
                if (CheckCollisionCircleRec(collectible.getCenter(), collectible.getRadius(), turret.getRectangle())) {
                    if (collectible.isGoodBall) {
                        continue;
                    } else if (!collectible.isGoodBall) {
                        if (random_25_percent_chance(rng) == 1) {
                            score++;
                        }
                        collectible.shouldBeDestroyed = true;
                    }
                }
            }
        }
        
    }
};

class SpecialCollectible {
public: 
    int radius = 25;
    int centerX, centerY;
    bool isGoodBall;
    bool shouldBeDestroyed = false;
    
    SpecialCollectible() {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<int> distX(50, GetScreenWidth() - 50);
        centerX = distX(rng);
        centerY = 100;
    }
    void paint() {
        DrawCircle(centerX, centerY, radius, YELLOW);
    }
    void Update() {
        if (centerY - radius < WINDOW_HEIGHT) {
            centerY += velocity;
        }
        if (centerY - radius >= WINDOW_HEIGHT) {
            shouldBeDestroyed = true;
        }
    }
    int getRadius() {
        return (float)radius;
    }
    Vector2 getCenter() {
        Vector2 center = {(float)centerX, (float)centerY};
        return center;
    }
};

class SpawnSpecialCollectibles {
public: 
    std::vector<SpecialCollectible> special_collectibles = {};
    SpawnSpecialCollectibles() {

    }
    void addToSpecialCollectibles() {
        special_collectibles.push_back(SpecialCollectible());
    }
    void paint() {
        for (auto &special_collectible: special_collectibles) {
            special_collectible.Update();
            special_collectible.paint();
        }
    }
    void CheckForRemoval() {
        for (int i = 0; i < special_collectibles.size(); i++) {
            if (special_collectibles.at(i).shouldBeDestroyed) {
                special_collectibles.erase(special_collectibles.begin() + i);
                i--;
            }
        }
    }
    void CheckForCollisions(Collector collector) {
        for (auto &special_collectible: special_collectibles) {
            if (CheckCollisionCircleRec(special_collectible.getCenter(), special_collectible.getRadius(), collector.getRectangle())) {
                static std::mt19937 rng(std::random_device{}());
                static std::bernoulli_distribution randomBool(0.5);
                bool chance = randomBool(rng);
                if (chance) {
                    collectorsHaveProtection = true;
                    collectorsHaveProtectionAtomic = true;
                } else if (!chance) {
                    collectorsHaveSpeedBoost = true;
                    collectorsHaveSpeedBoostAtomic = true;
                }
                
                special_collectible.shouldBeDestroyed = true;
            }
        }
        
    }
};

SpawnCollectibles spawn_collectibles = SpawnCollectibles();
SpawnTurrets spawn_turrets = SpawnTurrets();
SpawnSpecialCollectibles spawn_special_collectibles = SpawnSpecialCollectibles();

int main() {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> special_collectible_chance(1, 4);
    std::thread t(Timer);
    std::thread ProtectionTimerThread(ProtectionTimer);
    std::thread SpeedBoostTimerThread(SpeedBoostTimer);
    int tick_count = 0;
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Collect the Blocks!");
    SetTargetFPS(60);
    Collector collector = Collector();
    while (!WindowShouldClose()) {
        BeginDrawing();
        if (IsKeyDown(KEY_A)) {
            collector.moveLeft();
        }
        if (IsKeyDown(KEY_D)) {
            collector.moveRight();
        }
        if (IsKeyPressed(KEY_SPACE)) {
            spawn_turrets.addToTurrets(collector);
        }
        ClearBackground(CUSTOM_BLUE);
        char score_label[16];
        sprintf(score_label, "Score: %d", score);
        DrawText(score_label, 50, 50, 40, BLACK);
        DrawText("Press A and D to move", GetScreenWidth() - 400, 50, 30, BLACK);
        DrawText("Press Space to shoot", GetScreenWidth() - 400, 90, 30, BLACK);
        collector.paint();
        if (timerTick) {
            timerTick = false;
            tick_count++;
            spawn_collectibles.SetSpawnRate();
            spawn_collectibles.addToCollectibles();
            int chance = special_collectible_chance(rng);
            if (chance == 1) {
                spawn_special_collectibles.addToSpecialCollectibles();
            }
        }
        spawn_collectibles.spawn();
        spawn_turrets.drawTurret();
        spawn_turrets.Update();
        spawn_turrets.checkForRemoveableTurrets();
        spawn_special_collectibles.paint();
        spawn_special_collectibles.CheckForRemoval();
        spawn_collectibles.CheckForRemoval();
        spawn_collectibles.CheckForCollision(collector, spawn_turrets);
        spawn_special_collectibles.CheckForCollisions(collector);
        EndDrawing();
    }
    
    CloseWindow();
    std::exit(0);
    running = false;
    t.join();
    SpeedBoostTimerThread.join();
    ProtectionTimerThread.join();
    return 0;
}
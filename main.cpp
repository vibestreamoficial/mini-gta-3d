#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

// ==================== CONFIG ====================
const int SCREEN_WIDTH  = 1280;
const int SCREEN_HEIGHT = 720;
const float PLAYER_SPEED = 8.0f;
const float CAR_SPEED    = 18.0f;
const float GRAVITY      = 20.0f;

// ==================== STRUCTS ====================
struct Building {
    Vector3 position;
    Vector3 size;
    Color color;
};

struct NPC {
    Vector3 position;
    Vector3 target;
    float speed;
    Color color;
    float timer;
};

struct Car {
    Vector3 position;
    float yaw;          // rotação em graus
    bool occupied;
    Color color;
};

// ==================== GLOBAL ====================
Camera3D camera = { 0 };
Vector3 playerPos = { 0.0f, 1.0f, 0.0f };
float playerYaw = 0.0f;
float playerPitch = 0.0f;
float playerVelY = 0.0f;
bool inCar = false;
bool onGround = true;

std::vector<Building> buildings;
std::vector<NPC> npcs;
Car car;

// ==================== FUNÇÕES ====================
void GenerateCity() {
    // Chão grande
    // Prédios em grid
    for (int x = -6; x <= 6; x++) {
        for (int z = -6; z <= 6; z++) {
            if (x == 0 && z == 0) continue; // centro livre
            if (rand() % 3 == 0) continue; // alguns vazios (ruas)

            float height = 4.0f + (rand() % 12);
            Building b;
            b.position = { (float)x * 12.0f, height / 2.0f, (float)z * 12.0f };
            b.size = { 8.0f + (rand() % 4), height, 8.0f + (rand() % 4) };
            b.color = Color{
                (unsigned char)(40 + rand() % 80),
                (unsigned char)(40 + rand() % 80),
                (unsigned char)(50 + rand() % 100),
                255
            };
            buildings.push_back(b);
        }
    }
}

void SpawnNPCs(int count) {
    for (int i = 0; i < count; i++) {
        NPC n;
        n.position = {
            (float)(rand() % 100 - 50),
            1.0f,
            (float)(rand() % 100 - 50)
        };
        n.target = n.position;
        n.speed = 2.0f + (rand() % 30) / 10.0f;
        n.color = Color{
            (unsigned char)(100 + rand() % 155),
            (unsigned char)(100 + rand() % 155),
            (unsigned char)(100 + rand() % 155),
            255
        };
        n.timer = 0.0f;
        npcs.push_back(n);
    }
}

bool CheckCollisionBuildings(Vector3 pos, float radius) {
    for (const auto& b : buildings) {
        BoundingBox box = {
            { b.position.x - b.size.x/2, 0, b.position.z - b.size.z/2 },
            { b.position.x + b.size.x/2, b.size.y, b.position.z + b.size.z/2 }
        };
        if (CheckCollisionBoxSphere(box, pos, radius)) return true;
    }
    return false;
}

void UpdateNPCs(float dt) {
    for (auto& n : npcs) {
        n.timer -= dt;
        if (n.timer <= 0.0f) {
            n.target = {
                n.position.x + (rand() % 40 - 20),
                1.0f,
                n.position.z + (rand() % 40 - 20)
            };
            n.timer = 3.0f + (rand() % 40) / 10.0f;
        }

        Vector3 dir = Vector3Subtract(n.target, n.position);
        float dist = Vector3Length(dir);
        if (dist > 0.5f) {
            dir = Vector3Scale(Vector3Normalize(dir), n.speed * dt);
            Vector3 next = Vector3Add(n.position, dir);
            if (!CheckCollisionBuildings(next, 0.6f)) {
                n.position = next;
            } else {
                n.timer = 0.0f; // escolhe novo destino
            }
        }
    }
}

void UpdatePlayer(float dt) {
    if (inCar) return;

    // Mouse look
    Vector2 mouseDelta = GetMouseDelta();
    playerYaw -= mouseDelta.x * 0.15f;
    playerPitch -= mouseDelta.y * 0.15f;
    if (playerPitch > 89.0f) playerPitch = 89.0f;
    if (playerPitch < -89.0f) playerPitch = -89.0f;

    // Movimento
    Vector3 forward = {
        sinf(DEG2RAD * playerYaw),
        0.0f,
        cosf(DEG2RAD * playerYaw)
    };
    Vector3 right = {
        cosf(DEG2RAD * playerYaw),
        0.0f,
        -sinf(DEG2RAD * playerYaw)
    };

    Vector3 move = { 0 };
    if (IsKeyDown(KEY_W)) move = Vector3Add(move, forward);
    if (IsKeyDown(KEY_S)) move = Vector3Subtract(move, forward);
    if (IsKeyDown(KEY_A)) move = Vector3Subtract(move, right);
    if (IsKeyDown(KEY_D)) move = Vector3Add(move, right);

    if (Vector3Length(move) > 0.0f) {
        move = Vector3Normalize(move);
        move = Vector3Scale(move, PLAYER_SPEED * dt);
        Vector3 next = Vector3Add(playerPos, move);
        if (!CheckCollisionBuildings(next, 0.5f)) {
            playerPos.x = next.x;
            playerPos.z = next.z;
        }
    }

    // Pulo e gravidade
    if (IsKeyPressed(KEY_SPACE) && onGround) {
        playerVelY = 8.0f;
        onGround = false;
    }
    playerVelY -= GRAVITY * dt;
    playerPos.y += playerVelY * dt;

    if (playerPos.y <= 1.0f) {
        playerPos.y = 1.0f;
        playerVelY = 0.0f;
        onGround = true;
    }
}

void UpdateCar(float dt) {
    if (!inCar) return;

    // Mouse look também no carro
    Vector2 mouseDelta = GetMouseDelta();
    playerYaw -= mouseDelta.x * 0.12f;

    float steer = 0.0f;
    if (IsKeyDown(KEY_A)) steer = 1.0f;
    if (IsKeyDown(KEY_D)) steer = -1.0f;

    float accel = 0.0f;
    if (IsKeyDown(KEY_W)) accel = 1.0f;
    if (IsKeyDown(KEY_S)) accel = -0.6f;

    car.yaw += steer * 90.0f * dt * (accel != 0 ? 1.0f : 0.3f);

    Vector3 forward = {
        sinf(DEG2RAD * car.yaw),
        0.0f,
        cosf(DEG2RAD * car.yaw)
    };

    Vector3 move = Vector3Scale(forward, accel * CAR_SPEED * dt);
    Vector3 next = Vector3Add(car.position, move);

    if (!CheckCollisionBuildings(next, 1.5f)) {
        car.position = next;
    }

    // Player segue o carro
    playerPos = car.position;
    playerPos.y = 1.2f;
}

void UpdateCamera() {
    float camDistance = inCar ? 10.0f : 7.0f;
    float camHeight   = inCar ? 4.0f : 3.5f;

    Vector3 target = inCar ? car.position : playerPos;
    target.y += 1.0f;

    camera.target = target;
    camera.position.x = target.x - sinf(DEG2RAD * playerYaw) * camDistance;
    camera.position.z = target.z - cosf(DEG2RAD * playerYaw) * camDistance;
    camera.position.y = target.y + camHeight;
}

int main() {
    srand((unsigned)time(nullptr));

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Mini GTA 3D - Protótipo");
    SetTargetFPS(60);
    DisableCursor();

    // Câmera
    camera.position = { 0.0f, 5.0f, -10.0f };
    camera.target = { 0.0f, 1.0f, 0.0f };
    camera.up = { 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    GenerateCity();
    SpawnNPCs(12);

    // Carro inicial
    car.position = { 5.0f, 0.5f, 5.0f };
    car.yaw = 0.0f;
    car.occupied = false;
    car.color = RED;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Entrar / sair do carro
        if (IsKeyPressed(KEY_E)) {
            if (!inCar) {
                float dist = Vector3Distance(playerPos, car.position);
                if (dist < 4.0f) {
                    inCar = true;
                    car.occupied = true;
                }
            } else {
                inCar = false;
                car.occupied = false;
                // sai do lado do carro
                playerPos.x = car.position.x + 3.0f;
                playerPos.z = car.position.z;
                playerPos.y = 1.0f;
            }
        }

        UpdatePlayer(dt);
        UpdateCar(dt);
        UpdateNPCs(dt);
        UpdateCamera();

        // ========== DRAW ==========
        BeginDrawing();
        ClearBackground(Color{30, 30, 40, 255});

        BeginMode3D(camera);

        // Chão
        DrawPlane({0, 0, 0}, {200, 200}, Color{40, 40, 45, 255});
        DrawGrid(40, 5.0f);

        // Prédios
        for (const auto& b : buildings) {
            DrawCube(b.position, b.size.x, b.size.y, b.size.z, b.color);
            DrawCubeWires(b.position, b.size.x, b.size.y, b.size.z, BLACK);
        }

        // NPCs
        for (const auto& n : npcs) {
            DrawCapsule(n.position, {n.position.x, n.position.y + 1.6f, n.position.z}, 0.35f, 8, 8, n.color);
        }

        // Carro
        DrawCube(car.position, 2.2f, 1.0f, 4.5f, car.color);
        DrawCubeWires(car.position, 2.2f, 1.0f, 4.5f, BLACK);
        // "faróis"
        Vector3 front = {
            car.position.x + sinf(DEG2RAD * car.yaw) * 2.3f,
            car.position.y + 0.3f,
            car.position.z + cosf(DEG2RAD * car.yaw) * 2.3f
        };
        DrawSphere(front, 0.2f, YELLOW);

        // Player (só quando está a pé)
        if (!inCar) {
            DrawCapsule(playerPos, {playerPos.x, playerPos.y + 1.7f, playerPos.z}, 0.4f, 8, 8, SKYBLUE);
        }

        EndMode3D();

        // UI
        DrawRectangle(10, 10, 320, 110, Color{0, 0, 0, 180});
        DrawText("MINI GTA 3D - Prototipo", 20, 20, 20, RAYWHITE);
        DrawText("WASD = mover | Mouse = olhar", 20, 50, 16, LIGHTGRAY);
        DrawText("E = entrar/sair do carro", 20, 70, 16, LIGHTGRAY);
        DrawText(inCar ? "Status: DENTRO DO CARRO" : "Status: A PE", 20, 95, 16, inCar ? GREEN : SKYBLUE);

        DrawFPS(SCREEN_WIDTH - 90, 10);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

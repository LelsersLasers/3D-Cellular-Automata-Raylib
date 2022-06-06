#include "raylib.h"
#include <math.h>
#include <time.h>

#include <iostream>
// using namespace std;

#define PI 3.14159265358979323846

#define CELL_SIZE 1.0f
#define CELL_BOUNDS 30
#define aliveChanceOnSpawn 0.2


/*
Rules/explaination:
- Survival:
    - If a cell is alive, it will remain alive if it has A neighbors
- Spawn:
    - If a cell has B neighbors, it will come alive if dead
- State:
    - Once a cell begins dying, it has C game ticks to live before disappearing
    - Nothing can keep the cell from dying (even if neighbors change)
- Neighbor:
    - [M]oore: counts diagonal neighbors (3^3 - 1 = 26 possible neighbors)
    - [V]on [N]euman: only counts neighors where the faces touch
*/

enum NeighborType {
    MOORE,
    VON_NEUMAN
};

int SURVIVAL[] = { 4 };
int SPAWN[] = { 4 };
int STATE = 5;
NeighborType NEIGHBOR = MOORE;


enum State {
    ALIVE,
    DEAD,
    DYING
};


class ToggleKey {
private:
    bool wasDown = false;
public:
    bool down(bool pressState) {
        if (!wasDown && pressState) {
            wasDown = true;
            return true;
        }
        else if (!pressState) wasDown = false;
        return false;
    } 
};


class Cell {
private:
    State state;
    Vector3 pos = { 0.0f, 0.0f, 0.0f }; // center of cube, make sure to call setPos() before using
    int hp = STATE;
    int neighbors = 0;
public:
    Cell() { randomizeState(); }
    void setPos(Vector3 pos) { this->pos = pos; }
    void clearNeighbors() { neighbors = 0; }
    void addNeighbor(State neighborState) { neighbors += neighborState == ALIVE ? 1 : 0; }
    State getState() { return state; }
    void randomizeState() {
        state = (double)rand() / (double)RAND_MAX < aliveChanceOnSpawn ? ALIVE : DEAD;
        hp = STATE;
        neighbors = 0;
    }
    void sync() {
        switch (state) {
            case ALIVE: {
                bool willDie = true;
                for (int value : SURVIVAL) {
                    if (neighbors == value) {
                        willDie = false;
                        break;
                    }
                }
                if (willDie) state = DYING;
                break;
            }
            case DEAD: {
                for (int value : SPAWN) {
                    if (neighbors == value) {
                        state = ALIVE;
                        hp = STATE;
                        break;
                    }
                }
                break;
            }
            case DYING: {
                hp--;
                if (hp == 0) state = DEAD;
                break;
            }
        }
    }

    void draw() {
        if (this->state != DEAD) {
            Color color = RED;
            if (this->state == DYING) {
                unsigned char brightness = (int) ((float)this->hp/STATE * 255.0f);
                // unsigned char brightness = (int) ((26.0f - (float)neighbors)/26 * 255.0f);
                color = (Color){ brightness, brightness, brightness, 255 };
            }
            DrawCube(this->pos, CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
        }
    }
    void draw(Color color) {
        this->draw();
        if (this->state != DEAD) {
            DrawCubeWires(this->pos, CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
        }
    }

};


float degreesToRadians(float degrees) {
    return degrees * PI / 180.0f;
}

void drawCells(Cell cells[CELL_BOUNDS][CELL_BOUNDS][CELL_BOUNDS]) {
    for (int x = 0; x < CELL_BOUNDS; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y][z].draw(GREEN);
            }
        }
    }
    int outlineSize = CELL_SIZE * CELL_BOUNDS;
    DrawCubeWires((Vector3){ 0, 0, 0 }, outlineSize, outlineSize, outlineSize, BLUE);
}

void updateCells(Cell cells[CELL_BOUNDS][CELL_BOUNDS][CELL_BOUNDS]) {
    for (int x = 0; x < CELL_BOUNDS; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y][z].clearNeighbors();
                int offset_options[] = { -1, 0, 1 };
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        for (int k = 0; k < 3; k++) {
                            if (!(i == 0 && j == 0 && k == 0) &&
                                x + offset_options[i] >= 0 && x + offset_options[i] < CELL_BOUNDS &&
                                y + offset_options[j] >= 0 && y + offset_options[j] < CELL_BOUNDS &&
                                z + offset_options[k] >= 0 && z + offset_options[k] < CELL_BOUNDS) {
                                cells[x][y][z].addNeighbor(cells[x + offset_options[i]][y + offset_options[j]][z + offset_options[k]].getState());
                            }
                        }
                    }
                }
            }
        }
    }
    for (int x = 0; x < CELL_BOUNDS; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y][z].sync();
            }
        }
    }
}

void randomizeCells(Cell cells[CELL_BOUNDS][CELL_BOUNDS][CELL_BOUNDS]) {
    for (int x = 0; x < CELL_BOUNDS; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y][z].randomizeState();
            }
        }
    }
}


int main(void) {

    srand(time(NULL));

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "3D Cellular Automata");

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    float cameraLat = 20.0f;
    float cameraLon = 20.0f;
    float cameraRadius = 2.0f * CELL_SIZE * CELL_BOUNDS;
    float cameraMoveSpeed = 180.0f/4.0f;
    float cameraZoomSpeed = 10.0f;

    bool paused = false;

    ToggleKey mouseTK;
    ToggleKey spaceTK;
    ToggleKey xTK;
    ToggleKey zTK;

    int updateSpeed = 8;
    float frame = 0;


    Cell cells[CELL_BOUNDS][CELL_BOUNDS][CELL_BOUNDS]; // calls default consturctor
    for (int x = 0; x < CELL_BOUNDS; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y][z].setPos((Vector3){
                    CELL_SIZE * (x - (CELL_BOUNDS - 1.0f) / 2.0f),
                    CELL_SIZE * (y - (CELL_BOUNDS - 1.0f) / 2.0f),
                    CELL_SIZE * (z - (CELL_BOUNDS - 1.0f) / 2.0f)
                });
            }
        }
    }

    // Main game loop
    while (!WindowShouldClose()) {

        float delta = GetFrameTime();
        frame += delta;

        if (IsKeyDown('W') || IsKeyDown(KEY_UP)) cameraLat += cameraMoveSpeed * delta;
        else if (IsKeyDown('S') || IsKeyDown(KEY_DOWN)) cameraLat -= cameraMoveSpeed * delta;
        if (IsKeyDown('A') || IsKeyDown(KEY_LEFT)) cameraLon -= cameraMoveSpeed * delta;
        else if (IsKeyDown('D')|| IsKeyDown(KEY_RIGHT)) cameraLon += cameraMoveSpeed * delta;
        if (IsKeyDown('Q')) cameraRadius -= cameraZoomSpeed * delta;
        else if (IsKeyDown('E')) cameraRadius += cameraZoomSpeed * delta;

        if (IsKeyDown('R')) randomizeCells(cells);

        if (mouseTK.down(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) paused = !paused;
        if (xTK.down(IsKeyDown('X'))) updateSpeed++;
        if (zTK.down(IsKeyDown('Z'))) {
            if (updateSpeed > 1) updateSpeed--;
        };

        if (cameraLat > 90) cameraLat = 89.99f;
        else if (cameraLat < -90) cameraLat = -89.99f;

        if (cameraRadius < 1) cameraRadius = 1;

        camera.position = (Vector3){
            cameraRadius * cos(degreesToRadians(cameraLat)) * cos(degreesToRadians(cameraLon)),
            cameraRadius * cos(degreesToRadians(cameraLat)) * sin(degreesToRadians(cameraLon)),
            cameraRadius * sin(degreesToRadians(cameraLat))
        };

        if (!paused && frame >= 1.0f/updateSpeed) {
            updateCells(cells);
            while (frame >= 1.0/updateSpeed) frame -= 1.0/updateSpeed;
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
            drawCells(cells);
            EndMode3D();

            DrawRectangle( 10, 10, 320, 133, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines( 10, 10, 320, 133, BLUE);

            DrawText("Controls:", 20, 20, 10, BLACK);
            DrawText("- Q to zoom in, E to zoom out", 40, 40, 10, DARKGRAY);
            DrawText("- W to rotate camera up, S to rotate down", 40, 60, 10, DARKGRAY);
            DrawText("- A to rotate left, D to rotate right", 40, 80, 10, DARKGRAY);
            DrawText("- R to re-randomize the cells", 40, 100, 10, DARKGRAY);
            std::string fpsText = "- FPS: " + std::to_string(GetFPS());
            DrawText(fpsText.c_str(), 40, 120, 10, DARKGRAY);
            // draw text: info - fps, dist, camera coords, update speed, bounds, rules, etc

        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context
    return 0;
}
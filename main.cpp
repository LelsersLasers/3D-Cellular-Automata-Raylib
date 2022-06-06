#include "raylib.h"
#include <math.h>
#include <time.h>

// #include <iostream>
// using namespace std;

#define PI 3.14159265358979323846

#define CELL_SIZE 1.0f
#define CELL_BOUNDS 20


/*
Rules:
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



enum NeightborType {
    MOORE,
    VON_NEUMAN
};


int SURVIVAL[] = { 4 };
int SPAWN[] = { 4 };
int STATE = 5;
NeightborType NEIGHBOR = MOORE;



enum State {
    ALIVE,
    DEAD,
    DYING
};


class Cell {
// private:
    

public:

    State state;
    int hp;
    Vector3 pos; // center of cube
    float s; // size
    int neighbors = 0;

    Cell() {} // default

    Cell(State state, int hp, Vector3 pos, float s) {
        this->state = state;
        this->hp = hp;
        this->pos = pos;
        this->s = s;
    }

    void draw() {
        if (this->state != DEAD) {
            Color color = RED;
            if (this->state == DYING) {
                int brightness = (int) ((float)this->hp/STATE * 255.0f);
                color = (Color){ brightness, brightness, brightness, 255 };
            }
            DrawCube(this->pos, this->s, this->s, this->s, color);
        }
    }
    void draw(Color color) {
        this->draw();
        if (this->state != DEAD) {
            DrawCubeWires(this->pos, this->s, this->s, this->s, color);
        }
    }

};


float degreesToRadians(float degrees) {
    return degrees * PI / 180.0f;
}


int main(void) {

    srand(time(NULL));

    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "3D Cellular Automata");
    SetTargetFPS(3);

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    float cameraLat = 20.0f;
    float cameraLon = 20.0f;
    float cameraRadius = 30.0f;
    float cameraMoveSpeed = 1.0f * 20.0f;
    float cameraZoomSpeed = 0.25f * 20.0f;


    Cell cells[CELL_BOUNDS][CELL_BOUNDS][CELL_BOUNDS];
    for (int x = 0; x < CELL_BOUNDS; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y][z].state = (double)rand()/(double)RAND_MAX < 0.2 ? State::ALIVE : State::DEAD;
                cells[x][y][z].hp = STATE;
                cells[x][y][z].pos = (Vector3){
                    CELL_SIZE * (x - (CELL_BOUNDS - 1.0f) / 2.0f),
                    CELL_SIZE * (y - (CELL_BOUNDS - 1.0f) / 2.0f),
                    CELL_SIZE * (z - (CELL_BOUNDS - 1.0f) / 2.0f)
                };
                cells[x][y][z].s = CELL_SIZE;
            }
        }
    }

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        if (IsKeyDown('W')) cameraLat += cameraMoveSpeed;
        else if (IsKeyDown('S')) cameraLat -= cameraMoveSpeed;
        if (IsKeyDown('A')) cameraLon -= cameraMoveSpeed;
        else if (IsKeyDown('D')) cameraLon += cameraMoveSpeed;
        if (IsKeyDown('Q')) cameraRadius -= cameraZoomSpeed;
        else if (IsKeyDown('E')) cameraRadius += cameraZoomSpeed;

        if (cameraLat > 90) cameraLat = 89.99f;
        else if (cameraLat < -90) cameraLat = -89.99f;

        if (cameraRadius < 1) cameraRadius = 1;

        camera.position = (Vector3){
            cameraRadius * cos(degreesToRadians(cameraLat)) * cos(degreesToRadians(cameraLon)),
            cameraRadius * cos(degreesToRadians(cameraLat)) * sin(degreesToRadians(cameraLon)),
            cameraRadius * sin(degreesToRadians(cameraLat))
        };

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
            
            for (int x = 0; x < CELL_BOUNDS; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[x][y][z].draw(GREEN);
                    }
                }
            }

            DrawCubeWires((Vector3){ 0, 0, 0 }, CELL_SIZE * CELL_BOUNDS, CELL_SIZE * CELL_BOUNDS, CELL_SIZE * CELL_BOUNDS, BLUE);

            EndMode3D();

            DrawRectangle( 10, 10, 320, 133, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines( 10, 10, 320, 133, BLUE);

            DrawText("Camera controls:", 20, 20, 10, BLACK);
            DrawText("- Q to zoom in, E to zoom out", 40, 40, 10, DARKGRAY);
            DrawText("- W to rotate camera up, S to rotate down", 40, 60, 10, DARKGRAY);
            DrawText("- A to rotate left, D to rotate right", 40, 80, 10, DARKGRAY);
            // DrawText("- Alt + Ctrl + Mouse Wheel Pressed for Smooth Zoom", 40, 100, 10, DARKGRAY);
            // DrawText("- Z to zoom to (0, 0, 0)", 40, 120, 10, DARKGRAY);

        EndDrawing();

        int offset_options[] = { -1, 0, 1 };
        for (int x = 0; x < CELL_BOUNDS; x++) {
            for (int y = 0; y < CELL_BOUNDS; y++) {
                for (int z = 0; z < CELL_BOUNDS; z++) {
                    cells[x][y][z].neighbors = 0;
                    for (int i = 0; i < 3; i++) {
                        for (int j = 0; j < 3; j++) {
                            for (int k = 0; k < 3; k++) {
                                bool sameCell = i == 0 && j == 0 && k == 0;
                                if (!sameCell &&
                                    x + offset_options[i] >= 0 && x + offset_options[i] < CELL_BOUNDS &&
                                    y + offset_options[j] >= 0 && y + offset_options[j] < CELL_BOUNDS &&
                                    z + offset_options[k] >= 0 && z + offset_options[k] < CELL_BOUNDS) {
                                    cells[x][y][z].neighbors += cells[x + offset_options[i]][y + offset_options[j]][z + offset_options[k]].state == State::ALIVE ? 1 : 0;
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
                    switch (cells[x][y][z].state) {
                        case ALIVE: {
                            bool willDie = true;
                            for (int value : SURVIVAL) {
                                if (cells[x][y][z].neighbors == value) {
                                    willDie = false;
                                    break;
                                }
                            }
                            if (willDie) {
                                cells[x][y][z].state = DYING;
                            }
                            break;
                        }
                        case DEAD: {
                            for (int value : SPAWN) {
                                if (cells[x][y][z].neighbors == value) {
                                    cells[x][y][z].state = ALIVE;
                                    cells[x][y][z].hp = STATE;
                                    break;
                                }
                            }
                            break;
                        }
                        case DYING: {
                            cells[x][y][z].hp--;
                            if (cells[x][y][z].hp == 0) {
                                cells[x][y][z].state = DEAD;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    CloseWindow();        // Close window and OpenGL context
    return 0;
}
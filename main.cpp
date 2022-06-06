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
    Color color;

    Cell() {} // default

    Cell(State state, int hp, Vector3 pos, float s, Color color) {
        this->state = state;
        this->hp = hp;
        this->pos = pos;
        this->s = s;
        this->color = color;
    }

    void draw() {
        if (this->state != DEAD) {
            DrawCube(this->pos, this->s, this->s, this->s, this->color);
        }
    }
    void draw(Color color) {
        this->draw();
        DrawCubeWires(this->pos, this->s, this->s, this->s, color);
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
    SetTargetFPS(60);

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    float cameraLat = 20.0f;
    float cameraLon = 20.0f;
    float cameraRadius = 20.0f;
    float cameraMoveSpeed = 1.0f;
    float cameraZoomSpeed = 0.25f;


    Cell cells[CELL_BOUNDS];
    for (int i = 0; i < CELL_BOUNDS; i++) {
        cells[i].state = (double)rand()/(double)RAND_MAX < 0.5 ? State::ALIVE : State::DEAD;
        cells[i].hp = 5;
        cells[i].pos = (Vector3){ CELL_SIZE * (i - (CELL_BOUNDS - 1.0f) / 2.0f), 0.0f, 0.0f };
        cells[i].s = CELL_SIZE;
        cells[i].color = (Color){ 255, 0, 0, 255 };
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

                for (Cell c : cells) {
                    c.draw(GREEN);
                }

                DrawCubeWires((Vector3){ 0.0f, 0.0f, 0.0f }, 2.0f, 2.0f, 2.0f, BLUE);

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
    }


    CloseWindow();        // Close window and OpenGL context
    return 0;
}
#include "raylib.h"
#include <math.h>
#include <time.h>
#include <vector>

#include <iostream>
using namespace std;

#define PI 3.14159265358979323846

#define CELL_SIZE 1.0f
#define CELL_BOUNDS 60
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
    - [V]on [N]euman: only counts neighors where the faces touch (6 possible)
*/

enum NeighborType {
    MOORE,
    VON_NEUMANN
};

int SURVIVAL[] = { 4 };
int SPAWN[] = { 4 };
int STATE = 5;
NeighborType NEIGHBORHOODS = MOORE;


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
    Vector3 pos;
    int hp = STATE;
    int neighbors = 0;
public:
    Cell(Vector3 pos) {
        this->pos = pos;
        randomizeState();
    }
    void clearNeighbors() { neighbors = 0; }
    
    void addNeighbor(State neighborState) { neighbors += neighborState == ALIVE ? 1 : 0; }
    State getState() const { return state; }
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

    void draw() const {
        if (this->state != DEAD) {
            Color color = RED;
            if (this->state == DYING) {
                float percent = (1 + (float)this->hp)/(STATE + 2);
                unsigned char brightness = (int)(percent * 255.0f);
                color = (Color){ brightness, brightness, brightness, 255 };
            }
            DrawCube(this->pos, CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
        }
    }
    // void draw(Color color) const {
    //     this->draw();
    //     if (this->state != DEAD) {
    //         DrawCubeWires(this->pos, CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
    //     }
    // }

};


float degreesToRadians(float degrees) {
    return degrees * PI / 180.0f;
}

string textFromEnum(NeighborType nt) {
    switch (nt) {
        case MOORE: return "Moore";
        case VON_NEUMANN: return "von Neumann";
    }
    return "";
}

void drawCells(const vector<vector<vector<Cell>>> &cells) {
    for (int x = 0; x < CELL_BOUNDS; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y][z].draw();
            }
        }
    }
    int outlineSize = CELL_SIZE * CELL_BOUNDS;
    DrawCubeWires((Vector3){ 0, 0, 0 }, outlineSize, outlineSize, outlineSize, BLUE);
}

void updateCells(vector<vector<vector<Cell>>> &cells) {
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

void randomizeCells(vector<vector<vector<Cell>>> &cells) {
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

    const int screenWidth = 1200;
    const int screenHeight = 675;

    InitWindow(screenWidth, screenHeight, "3D Cellular Automata");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

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
    float cameraZoomSpeed = 12.0f;

    bool paused = false;

    ToggleKey mouseTK;
    ToggleKey enterTK;
    ToggleKey xTK;
    ToggleKey zTK;

    int updateSpeed = 8;
    float frame = 0;

    vector<vector<vector<Cell>>> cells;
    for (int x = 0; x < CELL_BOUNDS; x++) {
        cells.push_back(vector<vector<Cell>>());
        for (int y = 0; y < CELL_BOUNDS; y++) {
            cells[x].push_back(vector<Cell>());
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y].push_back(Cell((Vector3){
                    CELL_SIZE * (x - (CELL_BOUNDS - 1.0f) / 2.0f),
                    CELL_SIZE * (y - (CELL_BOUNDS - 1.0f) / 2.0f),
                    CELL_SIZE * (z - (CELL_BOUNDS - 1.0f) / 2.0f)
                }));
            }
        }
    }
        

    // Main game loop
    while (!WindowShouldClose()) {

        float delta = GetFrameTime();
        frame += delta;

        if (IsKeyDown('W') || IsKeyDown(KEY_UP)) {
            cameraLat += cameraMoveSpeed * delta;
        }
        if (IsKeyDown('S') || IsKeyDown(KEY_DOWN)) {
            cameraLat -= cameraMoveSpeed * delta;
        }
        if (IsKeyDown('A') || IsKeyDown(KEY_LEFT)) {
            cameraLon -= cameraMoveSpeed * delta;
        }
        if (IsKeyDown('D')|| IsKeyDown(KEY_RIGHT)) {
            cameraLon += cameraMoveSpeed * delta;
        }
        if (IsKeyDown('Q') || IsKeyDown(KEY_PAGE_UP)) {
            cameraRadius -= cameraZoomSpeed * delta;
        }
        if (IsKeyDown('E') || IsKeyDown(KEY_PAGE_DOWN)) {
            cameraRadius += cameraZoomSpeed * delta;
        }
        if (IsKeyDown('R')) {
            randomizeCells(cells);
        }
        if (mouseTK.down(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) {
            paused = !paused;
        }
        if (xTK.down(IsKeyDown('X'))) {
            updateSpeed++;
        }
        if (zTK.down(IsKeyDown('Z'))) {
            if (updateSpeed > 1) updateSpeed--;
        }
        if (IsKeyDown(KEY_SPACE)) {
            cameraLat = 20.0f;
            cameraLon = 20.0f;
            cameraRadius = 2.0f * CELL_SIZE * CELL_BOUNDS;
        }
        if (enterTK.down(IsKeyPressed(KEY_ENTER))) {
            ToggleFullscreen();
            if (IsWindowFullscreen()) {
                int display = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
            }
            else {
                SetWindowSize(screenWidth, screenHeight);
            }
        }

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

            char dirs[2] = { 'N', 'W' };
            if (cameraLat < 0) {
                dirs[0] = 'S';
            }
            if (cameraLon < 0) {
                dirs[1] = 'E';
            }
            string survivalText = "- Survive:";
            for (int value : SURVIVAL) {
                survivalText += " " + to_string(value);
            }
            string spawnText = "- Spawn:";
            for (int value : SPAWN) {
                spawnText += " " + to_string(value);
            }
            // const char* fpsText = ("- FPS: " + to_string((int)(1.0f/delta))).c_str();
            // const char* cameraText = ("- Camera pos: " + to_string((int)abs(cameraLat)) + dirs[0] + ", " + to_string((int)abs(cameraLon)) + dirs[1]).c_str();
            // const char* texts[] = {
            //     "Controls:",
            //     "- Q/E to zoom in/out",
            //     "- W/S to rotate camera up/down",
            //     "- A/D to rotate camera left/right",
            //     "- R to randomize cells",
            //     "- X/Z to increase/decrease tick speed",
            //     "- Mouse click to pause/unpause",
            //     "- Space to reset camera",
            //     "Simulation Info:",
            //     fpsText,
            //     ("- Ticks per sec: " + to_string(updateSpeed)).c_str(),
            //     ("- Bound size: " + to_string(CELL_BOUNDS)).c_str(),
            //     cameraText,

            // };
            // int lenTexts = sizeof(texts) / sizeof(texts[0]);

            DrawRectangle( 10, 10, 270, 19 * 20 + 10, Fade(SKYBLUE, 0.5f));
            DrawRectangleLines( 10, 10, 270, 19 * 20 + 10, BLUE);

            // for (int i = 0; i < lenTexts; i++) {
            //     DrawText(texts[i], 20, 20 + i * 20, 10, DARKGRAY);
            // }

            DrawText("Controls:", 20, 20, 10, BLACK);
            DrawText("- Q/E to zoom in/out", 40, 40, 10, DARKGRAY);
            DrawText("- W/S to rotate camera up/down", 40, 60, 10, DARKGRAY);
            DrawText("- A/D to rotate camera left/right", 40, 80, 10, DARKGRAY);
            DrawText("- X/Z to increase/decrease tick speed", 40, 100, 10, DARKGRAY);
            DrawText("- Mouse click to pause/unpause", 40, 120, 10, DARKGRAY);
            DrawText("- R to re-randomize cells", 40, 140, 10, DARKGRAY);
            DrawText("- Space to reset camera", 40, 160, 10, DARKGRAY);
            DrawText("- Enter to toggle fullscreen", 40, 180, 10, DARKGRAY);

            DrawText("Simulation Info:", 20, 200, 10, BLACK);
            DrawText(("- FPS: " + to_string(GetFPS())).c_str(), 40, 220, 10, DARKGRAY);
            DrawText(("- Ticks per sec: " + to_string(updateSpeed)).c_str(), 40, 240, 10, DARKGRAY);
            DrawText(("- Bound size: " + to_string(CELL_BOUNDS)).c_str(), 40, 260, 10, DARKGRAY);
            DrawText(("- Camera pos: " + to_string((int)abs(cameraLat)) + dirs[0] + ", " + to_string((int)abs(cameraLon)) + dirs[1]).c_str(), 40, 280, 10, DARKGRAY);

            DrawText("Rules:", 20, 300, 10, BLACK);
            DrawText(survivalText.c_str(), 40, 320, 10, DARKGRAY);
            DrawText(spawnText.c_str(), 40, 340, 10, DARKGRAY);
            DrawText(("- State: " + to_string(STATE)).c_str(), 40, 360, 10, DARKGRAY);
            DrawText(("- Neighborhoods: " + textFromEnum(NEIGHBORHOODS)).c_str(), 40, 380, 10, DARKGRAY);

        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context
    return 0;
}
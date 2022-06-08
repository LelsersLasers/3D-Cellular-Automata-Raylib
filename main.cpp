#include "raylib.h"
#include <math.h>
#include <time.h>
#include <vector>

#include <iostream>
using namespace std;

#define PI 3.14159265358979323846

#define CELL_SIZE 1.0f
#define CELL_BOUNDS 60
#define aliveChanceOnSpawn 0.3f


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


bool SURVIVAL[27];
bool SPAWN[27];

int survival_numbers[] = { 4 };
int spawn_numbers[] = { 4 };

const int STATE = 5;
const NeighborType NEIGHBORHOODS = MOORE;


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


class DrawableText {
private:
    string text;
public:
    DrawableText(string text) {
        this->text = text;
    }
    void draw(int i) const {
        int x = 40;
        Color color = DARKGRAY;
        if (text[0] != '-') {
            x = 20;
            color = BLACK;
        }
        DrawText(text.c_str(), x, (i + 1) * 14, 10, color);
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
                if (!SURVIVAL[neighbors]) state = DYING;
                break;
            }
            case DEAD: {
                if (SPAWN[neighbors]) {
                    state = ALIVE;
                    hp = STATE;
                }
                break;
            }
            case DYING: {
                if (--hp == 0) state = DEAD;
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
            // unsigned char brightness = (int)((((float)this->hp)/STATE) * 255.0f);
            // Color color = (Color){ 0, brightness, 0, 255 };
            DrawCube(this->pos, CELL_SIZE, CELL_SIZE, CELL_SIZE, color);
        }
    }
};


void setupBoolArrays() {
    for (int value : survival_numbers) {
        SURVIVAL[value] = true;
    }
    for (int value : spawn_numbers) {
        SPAWN[value] = true;
    }
}

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


bool validCellIndex(int x, int y, int z, int a, int b, int c) {
    return x + a >= 0 && x + a < CELL_BOUNDS &&
           y + b >= 0 && y + b < CELL_BOUNDS &&
           z + c >= 0 && z + c < CELL_BOUNDS;
}

void updateNeighborsMoore(vector<vector<vector<Cell>>> &cells) {
    const int offset_options[] = { -1, 0, 1 };
    for (int x = 0; x < CELL_BOUNDS; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y][z].clearNeighbors();
                for (int a : offset_options) {
                    for (int b : offset_options) {
                        for (int c : offset_options) {
                            if (!(a == 0 && b == 0 && c == 0) && validCellIndex(x, y, z, a, b, c)) {
                                cells[x][y][z].addNeighbor(cells[x + a][y + b][z + c].getState());
                            }
                        }
                    }
                }
            }
        }
    }
}

void updateNeighborsVonNeumann(vector<vector<vector<Cell>>> &cells) {
    const Vector3 offsets[6] = {
        { 1, 0, 0 },
        { -1, 0, 0 },
        { 0, 1, 0 },
        { 0, -1, 0 },
        { 0, 0, 1 },
        { 0, 0, -1 }
    };
    for (int x = 0; x < CELL_BOUNDS; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y][z].clearNeighbors();
                for (Vector3 offset : offsets) {
                    if (validCellIndex(x, y, z, offset.x, offset.y, offset.z)) {
                        cells[x][y][z].addNeighbor(cells[x + offset.x][y + offset.y][z + offset.z].getState());
                    }
                }
            }
        }
    }
}

void updateCells(vector<vector<vector<Cell>>> &cells) {
    if (NEIGHBORHOODS == MOORE) updateNeighborsMoore(cells);
    else updateNeighborsVonNeumann(cells);
}


void drawAndSyncCells(vector<vector<vector<Cell>>> &cells) {
    for (int x = 0; x < CELL_BOUNDS; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y][z].sync();
                cells[x][y][z].draw();
            }
        }
    }
}

void basicDrawCells(const vector<vector<vector<Cell>>> &cells) {
    for (int x = 0; x < CELL_BOUNDS; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y][z].draw();
            }
        }
    }
}

void drawCells(vector<vector<vector<Cell>>> &cells, bool toSync, bool drawBounds) {
    if (toSync) drawAndSyncCells(cells);
    else basicDrawCells(cells);

    if (drawBounds) {
        int outlineSize = CELL_SIZE * CELL_BOUNDS;
        DrawCubeWires((Vector3){ 0, 0, 0 }, outlineSize, outlineSize, outlineSize, BLUE);
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


void drawLeftBar(float cameraLat, float cameraLon, bool paused, int updateSpeed) {
    char dirs[2] = {
        (cameraLat > 0 ? 'N' : 'S'),
        (cameraLon > 0 ? 'W' : 'E')
    };

    string survivalText = "- Survive:";
    for (int i = 0; i < 27; i++) {
        if (SURVIVAL[i]) survivalText += " " + to_string(i);
    }

    string spawnText = "- Spawn:";
    for (int i = 0; i < 27; i++) {
        if (SPAWN[i]) spawnText += " " + to_string(i);
    }

    const DrawableText dts[] = {
        DrawableText("Controls:"),
        DrawableText("- Q/E to zoom in/out"),
        DrawableText("- W/S to rotate camera up/down"),
        DrawableText("- A/D to rotate camera left/right"),
        DrawableText("- X/Z to increase/decrease tick speed"),
        DrawableText("- Mouse click to pause/unpause"),
        DrawableText("- R to re-randomize cells"),
        DrawableText("- B to show/hide bounds"),
        DrawableText("- Space to reset camera"),
        DrawableText("- Enter to toggle fullscreen"),

        DrawableText((string)"Simulation Info: " + (string)(paused ? "paused" : "running")),
        DrawableText("- FPS: " + to_string(GetFPS())),
        DrawableText("- Ticks per sec: " + to_string(updateSpeed)),
        DrawableText("- Bound size: " + to_string(CELL_BOUNDS)),
        DrawableText("- Camera pos: " + to_string((int)abs(cameraLat)) + dirs[0] + ", " + to_string(abs((int)cameraLon)) + dirs[1]),

        DrawableText("Rules:"),
        DrawableText(survivalText),
        DrawableText(spawnText),
        DrawableText("- State: " + to_string(STATE)),
        DrawableText("- Neighborhood: " + textFromEnum(NEIGHBORHOODS)),
    };

    const int lenTexts = sizeof(dts) / sizeof(dts[0]);

    DrawRectangle(10, 10, 270, lenTexts * 14 + 7, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(10, 10, 270, lenTexts * 14 + 7, BLUE);

    for (int i = 0; i < lenTexts; i++) {
        dts[i].draw(i);
    }
}


int main(void) {

    srand(time(NULL));

    const int screenWidth = 1200;
    const int screenHeight = 675;

    InitWindow(screenWidth, screenHeight, "3D Cellular Automata with Raylib");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    setupBoolArrays();

    Camera3D camera = { 0 };
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f }; // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type

    float cameraLat = 20.0f;
    float cameraLon = 20.0f;
    float cameraRadius = 1.75f * CELL_SIZE * CELL_BOUNDS;
    const float cameraMoveSpeed = 180.0f/4.0f;
    const float cameraZoomSpeed = 12.0f;

    bool paused = false;
    bool drawBounds = true;

    ToggleKey mouseTK;
    ToggleKey enterTK;
    ToggleKey xTK;
    ToggleKey zTK;
    ToggleKey bTK;

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

        const float delta = GetFrameTime();
        frame += delta;

        if (IsKeyDown('W') || IsKeyDown(KEY_UP)) cameraLat += cameraMoveSpeed * delta;
        if (IsKeyDown('S') || IsKeyDown(KEY_DOWN)) cameraLat -= cameraMoveSpeed * delta;
        if (IsKeyDown('A') || IsKeyDown(KEY_LEFT)) cameraLon -= cameraMoveSpeed * delta;
        if (IsKeyDown('D')|| IsKeyDown(KEY_RIGHT)) cameraLon += cameraMoveSpeed * delta;
        if (IsKeyDown('Q') || IsKeyDown(KEY_PAGE_UP)) cameraRadius -= cameraZoomSpeed * delta;
        if (IsKeyDown('E') || IsKeyDown(KEY_PAGE_DOWN)) cameraRadius += cameraZoomSpeed * delta;
        if (IsKeyDown('R')) randomizeCells(cells);
        if (mouseTK.down(IsMouseButtonPressed(MOUSE_LEFT_BUTTON))) paused = !paused;
        if (bTK.down(IsKeyPressed('B'))) drawBounds = !drawBounds;
        if (xTK.down(IsKeyDown('X'))) updateSpeed++;
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
            else SetWindowSize(screenWidth, screenHeight);
        }

        if (cameraLat > 90) cameraLat = 89.99f;
        else if (cameraLat < -90) cameraLat = -89.99f;
        if (cameraLon > 180) cameraLon -= 360;
        else if (cameraLon < -180) cameraLon += 360;

        if (cameraRadius < CELL_SIZE) cameraRadius = CELL_SIZE;
        camera.position = (Vector3){
            cameraRadius * cos(degreesToRadians(cameraLat)) * cos(degreesToRadians(cameraLon)),
            cameraRadius * cos(degreesToRadians(cameraLat)) * sin(degreesToRadians(cameraLon)),
            cameraRadius * sin(degreesToRadians(cameraLat))
        };

        bool toSync = false;
        if (!paused && frame >= 1.0f/updateSpeed) {
            updateCells(cells);
            toSync = true;
            while (frame >= 1.0/updateSpeed) frame -= 1.0/updateSpeed; // for pause and if updateSpeed > fps
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                drawCells(cells, toSync, drawBounds);
            EndMode3D();

            drawLeftBar(cameraLat, cameraLon, paused, updateSpeed);

        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context
    return 0;
}
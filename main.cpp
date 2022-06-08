#include "raylib.h"
#include <math.h>
#include <time.h>
#include <vector>

#include <iostream>
using namespace std;

#define PI 3.14159265358979323846

#define CELL_SIZE 1.0f
#define CELL_BOUNDS 66
#define aliveChanceOnSpawn 0.15f


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

enum State {
    ALIVE,
    DEAD,
    DYING
};

enum DrawMode {
    DUAL_COLOR_1 = 0,
    RGB_CUBE = 1,
    DUAL_COLOR_DYING = 2,
    SINGLE_COLOR = 3,
    CENTER_DIST = 4
};

enum TickMode {
    MANUAL = 0,
    FAST = 1,
    DYNAMIC = 2
};


bool SURVIVAL[27];
bool SPAWN[27];

// My rules 9-18/5-7,12-13,15/6/M
// const int survival_numbers[] = { 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
// const int spawn_numbers[] = { 5, 6, 7, 12, 13, 15 };
// const int STATE = 6;
// const NeighborType NEIGHBORHOODS = MOORE;

// Builder 1 (Jason Rampe) 2,6,9/4,6,8-9/10/M
const int survival_numbers[] = { 2, 6, 9 };
const int spawn_numbers[] = { 4, 6, 8, 9 };
const int STATE = 10;
const NeighborType NEIGHBORHOODS = MOORE;

const Color C1 = GREEN;
const Color C2 = RED;
const Vector3 COLOR_OFFSET1 = {
    ((float)C1.r - C2.r)/STATE,
    ((float)C1.g - C2.g)/STATE,
    ((float)C1.b - C2.b)/STATE,
};

const Color C3 = BLACK;
const Color C4 = LIGHTGRAY;
const Vector3 COLOR_OFFSET2 = {
    ((float)C3.r - C4.r)/STATE,
    ((float)C3.g - C4.g)/STATE,
    ((float)C3.b - C4.b)/STATE,
};

const int TargetFPS = 20;


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
        int x = 30;
        Color color = DARKGRAY;
        if (text[0] != '-') {
            x = 20;
            color = BLACK;
        }
        DrawText(text.c_str(), x, (i + 1) * 14, 10, color);
    }
    int length() const {
        return text.length();
    }
};

float calc_distance(Vector3 a, Vector3 b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2) + pow(a.z - b.z, 2));
}

class Cell {
private:
    State state;
    Vector3 pos;
    Vector3 index;
    int hp;
    int neighbors = 0;
    void draw(Color color) const { DrawCube(pos, CELL_SIZE, CELL_SIZE, CELL_SIZE, color); }
public:
    Cell(Vector3 index) {
        this->index = index;
        pos = (Vector3){
            CELL_SIZE * (index.x - (CELL_BOUNDS - 1.0f) / 2.0f),
            CELL_SIZE * (index.y - (CELL_BOUNDS - 1.0f) / 2.0f),
            CELL_SIZE * (index.z - (CELL_BOUNDS - 1.0f) / 2.0f)
        };
        randomizeState();
    }
    void clearNeighbors() { neighbors = 0; }
    void addNeighbor(State neighborState) { neighbors += neighborState == ALIVE ? 1 : 0; }
    State getState() const { return state; }
    void randomizeState() {
        if (index.x > CELL_BOUNDS/3.0f && index.x < CELL_BOUNDS * 2.0f/3.0f &&
            index.y > CELL_BOUNDS/3.0f && index.y < CELL_BOUNDS * 2.0f/3.0f &&
            index.z > CELL_BOUNDS/3.0f && index.z < CELL_BOUNDS * 2.0f/3.0f) { // only middle has spawn chance

            state = (double)rand() / (double)RAND_MAX < aliveChanceOnSpawn ? ALIVE : DEAD;
            if (state == ALIVE) hp = STATE;
            else hp = 0;
        }
        else {
            state = DEAD;
            hp = 0;
        }
        neighbors = 0;
    }
    void sync() {
        switch (state) {
            case ALIVE:
                if (!SURVIVAL[neighbors]) state = DYING;
                break;
            case DEAD:
                if (SPAWN[neighbors]) {
                    state = ALIVE;
                    hp = STATE;
                }
                break;
            case DYING:
                if (--hp == 0) state = DEAD;
                break;
        }
    }
    void drawDualColor1() const {
        if (state != DEAD) {
            draw((Color){
                (unsigned char)(C2.r + COLOR_OFFSET1.x * (float)hp),
                (unsigned char)(C2.g + COLOR_OFFSET1.y * (float)hp),
                (unsigned char)(C2.b + COLOR_OFFSET1.z * (float)hp),
                255
            });
        }
    }
    void drawDualColorDying() const {
        if (state != DEAD) {
            Color color = RED;
            if (state == DYING) {
                float percent = (1 + (float)hp)/(STATE + 2);
                unsigned char brightness = (int)(percent * 255.0f);
                color = (Color){ brightness, brightness, brightness, 255 };
            }
            draw(color);
        }
    }
    void drawSingleColor() const {
        if (state != DEAD) {
            float x = 3.0f;
            float base = x/(STATE + x);
            float percent = (float)hp/(STATE + x);
            unsigned char brightness = (int)((base + percent) * 255.0f);
            draw((Color){ brightness, 20, 20, 255 });
        }
    }
    void drawRGBCube() const {
        if (state != DEAD) {
            draw((Color){
                (unsigned char)(index.x/CELL_BOUNDS * 255.0f),
                (unsigned char)(index.y/CELL_BOUNDS * 255.0f),
                (unsigned char)(index.z/CELL_BOUNDS * 255.0f),
                255
            });
        }
    }
    void drawDist() const {
        if (state != DEAD) {
            float x = 3.0f;
            float cap = CELL_BOUNDS/2.0f;
            float dist = calc_distance(index, { cap, cap, cap });
            float base = x/(cap * sqrt(3.0f) + x);
            float percent = dist/(cap * sqrt(3.0f) + x);
            unsigned char brightness = (int)((base + percent) * 255.0f);
            draw((Color){ brightness, brightness, brightness, 255 });
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
string textFromEnum(DrawMode dm) {
    switch (dm) {
        case DUAL_COLOR_1: return "Dual Color 1";
        case DUAL_COLOR_DYING: return "Dual Color Dying";
        case SINGLE_COLOR: return "Single Color";
        case RGB_CUBE: return "RGB";
        case CENTER_DIST: return "Center Dist";
    }
    return "";
}
string textFromEnum(TickMode tm) {
    switch (tm) {
        case MANUAL: return "Manual";
        case FAST: return "Fastest";
        case DYNAMIC: return "Dynamic";
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


void drawAndSyncCells(vector<vector<vector<Cell>>> &cells, int divisor, DrawMode drawMode) {
    // A bit exessive to put this outside, but is saves doing CELL_BOUNDS^3 extra checks
    // at the cost of extra code
    switch (drawMode) {
        case DUAL_COLOR_1:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[x][y][z].sync();
                        cells[x][y][z].drawDualColor1();
                    }
                }
            }
            break;
        case DUAL_COLOR_DYING:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[x][y][z].sync();
                        cells[x][y][z].drawDualColorDying();
                    }
                }
            }
            break;
        case SINGLE_COLOR:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[x][y][z].sync();
                        cells[x][y][z].drawSingleColor();
                    }
                }
            }
            break;
        case RGB_CUBE:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[x][y][z].sync();
                        cells[x][y][z].drawRGBCube();
                    }
                }
            }
            break;
        case CENTER_DIST:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[x][y][z].sync();
                        cells[x][y][z].drawDist();
                    }
                }
            }
            break;
    }
    for (int x = CELL_BOUNDS/divisor; x < CELL_BOUNDS; x++) { // still sync second half
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y][z].sync();
            }
        }
    }
}

void basicDrawCells(const vector<vector<vector<Cell>>> &cells, int divisor, DrawMode drawMode) {
    // A bit exessive to put this outside, but is saves doing CELL_BOUNDS^3 extra checks
    // at the cost of extra code
    switch (drawMode) {
        case DUAL_COLOR_1:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[x][y][z].drawDualColor1();
                    }
                }
            }
            break;
        case DUAL_COLOR_DYING:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[x][y][z].drawDualColorDying();
                    }
                }
            }
            break;
        case SINGLE_COLOR:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[x][y][z].drawSingleColor();
                    }
                }
            }
            break;
        case RGB_CUBE:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[x][y][z].drawRGBCube();
                    }
                }
            }
            break;
        case CENTER_DIST:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[x][y][z].drawDist();
                    }
                }
            }
            break;
    }
}

void drawCells(vector<vector<vector<Cell>>> &cells, bool toSync, bool drawBounds, bool showHalf, DrawMode drawMode) {
    if (toSync) drawAndSyncCells(cells, (int)showHalf + 1, drawMode);
    else basicDrawCells(cells, (int)showHalf + 1, drawMode);

    if (drawBounds) {
        int outlineSize = CELL_SIZE * CELL_BOUNDS;
        if (showHalf) DrawCubeWires((Vector3){ (float)-outlineSize/4.0f, 0, 0 }, outlineSize/2, outlineSize, outlineSize, BLUE);
        else DrawCubeWires((Vector3){ 0, 0, 0 }, outlineSize, outlineSize, outlineSize, BLUE);
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


void drawLeftBar(bool drawBounds, bool showHalf, bool paused, DrawMode drawMode, TickMode tickMode, float cameraLat, float cameraLon, int updateSpeed) {
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
        DrawableText("- Q/E : zoom in/out"),
        DrawableText("- W/S : rotate camera up/down"),
        DrawableText("- A/D : rotate camera left/right"),
        DrawableText("- R : re-randomize cells"),
        DrawableText("- B : show/hide bounds " + (string)(drawBounds ? "(on)" : "(off)")),
        DrawableText("- C : toggle cross section view " + (string)(showHalf ? "(on)" : "(off)")),
        DrawableText("- Mouse click : pause/unpause " + (string)(paused ? "(paused)" : "(running)")),
        DrawableText("- Space : reset camera"),
        DrawableText("- Enter : toggle fullscreen"),
        DrawableText("- M : change between draw modes [" + textFromEnum(drawMode) + "]"),
        DrawableText("- U : change between tick modes [" + textFromEnum(tickMode) + "]"),

        DrawableText("Simulation Info:"),
        DrawableText("- FPS: " + to_string(GetFPS())),
        DrawableText("- Ticks per sec: " + to_string(tickMode == FAST ? GetFPS() : updateSpeed)),
        DrawableText("- Bound size: " + to_string(CELL_BOUNDS)),
        DrawableText("- Camera pos: " + to_string((int)abs(cameraLat)) + dirs[0] + ", " + to_string(abs((int)cameraLon)) + dirs[1]),

        DrawableText("Rules:"),
        DrawableText(survivalText),
        DrawableText(spawnText),
        DrawableText("- State: " + to_string(STATE)),
        DrawableText("- Neighborhood: " + textFromEnum(NEIGHBORHOODS)),
    };

    const int lenTexts = sizeof(dts) / sizeof(dts[0]);

    DrawRectangle(10, 10, 290, lenTexts * 14 + 7, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(10, 10, 290, lenTexts * 14 + 7, BLUE);

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
    camera.position = (Vector3){ 10.0f, 10.0f, 10.0f };
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 0.0f, 1.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    float cameraLat = 20.0f;
    float cameraLon = 20.0f;
    float cameraRadius = 1.75f * CELL_SIZE * CELL_BOUNDS;
    const float cameraMoveSpeed = 180.0f/4.0f;
    const float cameraZoomSpeed = CELL_SIZE * CELL_BOUNDS/10.0f;

    bool paused = false;
    bool drawBounds = true;
    bool showHalf = false;
    DrawMode drawMode = DUAL_COLOR_1;
    TickMode tickMode = MANUAL;

    ToggleKey mouseTK;
    ToggleKey enterTK;
    ToggleKey xTK;
    ToggleKey zTK;
    ToggleKey bTK;
    ToggleKey cTK;
    ToggleKey mTK;
    ToggleKey uTK;

    int updateSpeed = 8;
    float frame = 0;

    vector<vector<vector<Cell>>> cells;
    for (int x = 0; x < CELL_BOUNDS; x++) {
        cells.push_back(vector<vector<Cell>>());
        for (int y = 0; y < CELL_BOUNDS; y++) {
            cells[x].push_back(vector<Cell>());
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells[x][y].push_back(Cell({ (float)x, (float)y, (float)z }));
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
        if (xTK.down(IsKeyDown('X') && tickMode == MANUAL)) updateSpeed++;
        if (zTK.down(IsKeyDown('Z') && tickMode == MANUAL && updateSpeed > 1)) updateSpeed--;
        if (cTK.down(IsKeyDown('C'))) showHalf = !showHalf;
        if (mTK.down(IsKeyDown('M'))) drawMode = (DrawMode)((drawMode + 1) % 5);
        if (uTK.down(IsKeyDown('U'))) tickMode = (TickMode)((tickMode + 1) % 3);;
        if (IsKeyDown(KEY_SPACE)) {
            cameraLat = 20.0f;
            cameraLon = 20.0f;
            cameraRadius = 2.0f * CELL_SIZE * CELL_BOUNDS;
        }
        if (enterTK.down(IsKeyPressed(KEY_ENTER))) {
            if (GetScreenWidth() == screenWidth) MaximizeWindow();
            else RestoreWindow();
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
        if (!paused && (tickMode == FAST || frame >= 1.0f/updateSpeed)) {
            if (tickMode == DYNAMIC) {
                if (GetFPS() > TargetFPS && updateSpeed < GetFPS()) updateSpeed++;
                else if (GetFPS() < TargetFPS && updateSpeed > 1) updateSpeed--;
            }
            updateCells(cells);
            toSync = true;
            while (frame >= 1.0/updateSpeed) frame -= 1.0/updateSpeed;
        }

        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);
                drawCells(cells, toSync, drawBounds, showHalf, drawMode);
            EndMode3D();

            drawLeftBar(drawBounds, showHalf, paused, drawMode, tickMode, cameraLat, cameraLon, updateSpeed);

        EndDrawing();
    }

    CloseWindow();        // Close window and OpenGL context
    return 0;
}
#include "raylib.h"
#include <math.h>
#include <time.h>
#include <vector>
#include <thread>
#include <iostream>

using std::string;
using std::vector;
using std::thread;

#define PI 3.14159265358979323846f

#define CELL_SIZE 1.0f
#define CELL_BOUNDS 96 // cleaner if divisible by THREADS
#define TOTAL_CELLS 884736
#define aliveChanceOnSpawn 0.15f
#define THREADS 8


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
    ALIVE = 2,
    DEAD = 0,
    DYING = 1
};

enum DrawMode {
    DUAL_COLOR = 0,
    RGB_CUBE = 1,
    DUAL_COLOR_DYING = 2,
    SINGLE_COLOR = 3,
    CENTER_DIST = 4
};

enum TickMode {
    FAST = 0,
    DYNAMIC = 1,
    MANUAL = 2
};


struct Vector3Int {
    int x, y, z;
};

bool SURVIVAL[27];
bool SPAWN[27];

// My rules 9-18/5-7,12-13,15/6/M
// const size_t survival_numbers[] = { 9, 10, 11, 12, 13, 14, 15, 16, 17, 18 };
// const size_t spawn_numbers[] = { 5, 6, 7, 12, 13, 15 };
// const int STATE = 6;
// const NeighborType NEIGHBORHOODS = MOORE;

// Builder 1 (Jason Rampe) 2,6,9/4,6,8-9/10/M
const size_t survival_numbers[] = { 2, 6, 9 };
const size_t spawn_numbers[] = { 4, 6, 8, 9 };
const int STATE = 10;
const NeighborType NEIGHBORHOODS = MOORE;

const Color C1 = GREEN;
const Color C2 = RED;
const Vector3 COLOR_OFFSET1 = {
    (float)(C1.r - C2.r)/STATE,
    (float)(C1.g - C2.g)/STATE,
    (float)(C1.b - C2.b)/STATE,
};

const Color C3 = BLACK;
const Color C4 = LIGHTGRAY;
const Vector3 COLOR_OFFSET2 = {
    (float)(C3.r - C4.r)/STATE,
    (float)(C3.g - C4.g)/STATE,
    (float)(C3.b - C4.b)/STATE,
};

const int TargetFPS = 8;


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
    DrawableText(string text) : text(text) {}
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

float calc_distance(Vector3Int a, Vector3Int b) {
    return sqrt(pow((float)a.x - b.x, 2) + pow((float)a.y - b.y, 2) + pow((float)a.z - b.z, 2));
}

class Cell {
private:
    State state;
    Vector3 pos;
    Vector3Int index;
    int hp;
    size_t neighbors = 0;
    void draw(Color color) const { DrawCube(pos, CELL_SIZE, CELL_SIZE, CELL_SIZE, color); }
public:
    Cell(Vector3Int index) {
        this->index = index;
        pos = {
            CELL_SIZE * (index.x - (CELL_BOUNDS - 1.0f) / 2),
            CELL_SIZE * (index.y - (CELL_BOUNDS - 1.0f) / 2),
            CELL_SIZE * (index.z - (CELL_BOUNDS - 1.0f) / 2)
        };
        randomizeState();
    }
    void clearNeighbors() { neighbors = 0; }
    void addNeighbor(int neighborState) { neighbors += neighborState/2; }
    State getState() const { return state; }
    void randomizeState() {
        if (index.x > CELL_BOUNDS/3.0f && index.x < CELL_BOUNDS * 2.0f/3.0f &&
            index.y > CELL_BOUNDS/3.0f && index.y < CELL_BOUNDS * 2.0f/3.0f &&
            index.z > CELL_BOUNDS/3.0f && index.z < CELL_BOUNDS * 2.0f/3.0
        ) { // only middle has spawn chance
            state = (double)rand() / (double)RAND_MAX < aliveChanceOnSpawn ? ALIVE : DEAD; // could branchless?
            // if (state == ALIVE) hp = STATE; // old
            hp = (int)(state/2.0f * STATE); // new
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
                // if (!SURVIVAL[neighbors]) state = DYING; // old
                state = (State)((int)(SURVIVAL[neighbors]) + 1); // new
                break;
            case DEAD:
                // if (SPAWN[neighbors]) { // old
                //     state = ALIVE;
                //     hp = STATE;
                // }
                state = (State)((int)(SPAWN[neighbors]) * 2);  // new
                hp = (int)(state/2.0f * STATE);

                break;
            case DYING:
                if (--hp == 0) state = DEAD;
                break;
        }
    }
    void drawDualColor() const {
        if (state != DEAD) {
            draw((Color){
                (unsigned char)(C2.r + COLOR_OFFSET1.x * hp),
                (unsigned char)(C2.g + COLOR_OFFSET1.y * hp),
                (unsigned char)(C2.b + COLOR_OFFSET1.z * hp),
                255
            });
        }
    }
    void drawDualColorDying() const {
        if (state != DEAD) {
            Color color = RED;
            if (state == DYING) {
                float percent = (1.0f + hp)/(STATE + 2.0f);
                unsigned char brightness = (int)(percent * 255);
                color = (Color){ brightness, brightness, brightness, 255 };
            }
            draw(color);
        }
    }
    void drawSingleColor() const {
        if (state != DEAD) {
            float x = 3.0f;
            float base = x/(STATE + x);
            float percent = hp/(STATE + x);
            unsigned char brightness = (int)((base + percent) * 255);
            draw((Color){ brightness, 20, 20, 255 });
        }
    }
    void drawRGBCube() const {
        if (state != DEAD) {
            draw((Color){
                (unsigned char)((float)index.x/CELL_BOUNDS * 255),
                (unsigned char)((float)index.y/CELL_BOUNDS * 255),
                (unsigned char)((float)index.z/CELL_BOUNDS * 255),
                255
            });
        }
    }
    void drawDist() const {
        if (state != DEAD) {
            float x = 3.0f;
            int cap = CELL_BOUNDS/2;
            float dist = calc_distance(index, { cap, cap, cap });
            float base = x/(cap * sqrt(3.0f) + x);
            float percent = dist/(cap * sqrt(3.0f) + x);
            unsigned char brightness = (int)((base + percent) * 255);
            draw((Color){ brightness, brightness, brightness, 255 });
        }
    }
};


string textFromEnum(NeighborType nt) {
    switch (nt) {
        case MOORE: return "Moore";
        case VON_NEUMANN: return "von Neumann";
    }
    return "";
}
string textFromEnum(DrawMode dm) {
    switch (dm) {
        case DUAL_COLOR: return "Dual Color";
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


void setupBoolArrays() {
    for (size_t value : survival_numbers) {
        SURVIVAL[value] = true;
    }
    for (size_t value : spawn_numbers) {
        SPAWN[value] = true;
    }
}

float degreesToRadians(float degrees) {
    return degrees * PI / 180.0f;
}

size_t threeToOne(int x, int y, int z) {
    return x * CELL_BOUNDS * CELL_BOUNDS + y * CELL_BOUNDS  + z;
}


bool validCellIndex(int x, int y, int z, const Vector3Int &offset) {
    return x + offset.x >= 0 && x + offset.x < CELL_BOUNDS &&
           y + offset.y >= 0 && y + offset.y < CELL_BOUNDS &&
           z + offset.z >= 0 && z + offset.z < CELL_BOUNDS;
}

void syncCells(vector<Cell> &cells, size_t start, size_t end) {
    for (size_t i = start; i < end; i++) {
        cells[i].sync();
    }
}

void updateNeighbors(vector<Cell> &cells, int start, int end, const Vector3Int offsets[], size_t totalOffsets) {
    for (int x = start; x < end; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                int oneIdx = threeToOne(x, y, z);
                cells[oneIdx].clearNeighbors();
                for (size_t i = 0; i < totalOffsets; i++) {
                    if (validCellIndex(x, y, z, offsets[i])) {
                        cells[oneIdx]
                            .addNeighbor(cells[threeToOne(x + offsets[i].x, y + offsets[i].y, z + offsets[i].z)]
                                .getState());
                    }
                }
            }
        }
    }
}

void updateCells(vector<Cell> &cells) {
    Vector3Int offsets[26];
    size_t totalOffsets;
    if (NEIGHBORHOODS == MOORE) {
        // const Vector3Int offsetsM[26] = {
        //     { -1, -1, -1 },
        //     { -1, -1, 0 },
        //     { -1, -1, 1 },
        //     { -1, 0, -1 },
        //     { -1, 0, 0 },
        //     { -1, 0, 1 },
        //     { -1, 1, -1 },
        //     { -1, 1, 0 },
        //     { -1, 1, 1 },
        //     { 0, -1, -1 },
        //     { 0, -1, 0 },
        //     { 0, -1, 1 },
        //     { 0, 0, -1 },
        //     { 0, 0, 1 },
        //     { 0, 1, -1 },
        //     { 0, 1, 0 },
        //     { 0, 1, 1 },
        //     { 1, -1, -1 },
        //     { 1, -1, 0 },
        //     { 1, -1, 1 },
        //     { 1, 0, -1 },
        //     { 1, 0, 0 },
        //     { 1, 0, 1 },
        //     { 1, 1, -1 },
        //     { 1, 1, 0 },
        //     { 1, 1, 1 }
        // };
        // idk how to hard set array so this was best I came up with
        offsets[0] = { -1, -1, -1 };
        offsets[1] = { -1, -1, 0 };
        offsets[2] = { -1, -1, 1 };
        offsets[3] = { -1, 0, -1 };
        offsets[4] = { -1, 0, 0 };
        offsets[5] = { -1, 0, 1 };
        offsets[6] = { -1, 1, -1 };
        offsets[7] = { -1, 1, 0 };
        offsets[8] = { -1, 1, 1 };
        offsets[9] = { 0, -1, -1 };
        offsets[10] = { 0, -1, 0 };
        offsets[11] = { 0, -1, 1 };
        offsets[12] = { 0, 0, -1 };
        offsets[13] = { 0, 0, 1 };
        offsets[14] = { 0, 1, -1 };
        offsets[15] = { 0, 1, 0 };
        offsets[16] = { 0, 1, 1 };
        offsets[17] = { 1, -1, -1 };
        offsets[18] = { 1, -1, 0 };
        offsets[19] = { 1, -1, 1 };
        offsets[20] = { 1, 0, -1 };
        offsets[21] = { 1, 0, 0 };
        offsets[22] = { 1, 0, 1 };
        offsets[23] = { 1, 1, -1 };
        offsets[24] = { 1, 1, 0 };
        offsets[25] = { 1, 1, 1 };
        totalOffsets = 26;
    }
    else {
        // const Vector3Int offsets[] = {
        //     { 1, 0, 0 },
        //     { -1, 0, 0 },
        //     { 0, 1, 0 },
        //     { 0, -1, 0 },
        //     { 0, 0, 1 },
        //     { 0, 0, -1 }
        // };
        offsets[0] = { 1, 0, 0 };
        offsets[1] = { -1, 0, 0 };
        offsets[2] = { 0, 1, 0 };
        offsets[3] = { 0, -1, 0 };
        offsets[4] = { 0, 0, 1 };
        offsets[5] = { 0, 0, -1 };
        totalOffsets = 6;
    }

    thread neighborThreads[THREADS];
    for (size_t i = 0; i < THREADS; i++) {
        int start = i * CELL_BOUNDS / THREADS;
        int end = (i + 1) * CELL_BOUNDS / THREADS;
        neighborThreads[i] = thread(updateNeighbors, std::ref(cells), start, end, offsets, totalOffsets);
    }
    for (size_t i = 0; i < THREADS; i++) {
        neighborThreads[i].join();
    }

    thread syncThreads[THREADS];
    for (size_t i = 0; i < THREADS; i++) {
        size_t start = i * TOTAL_CELLS / THREADS;
        size_t end = (i + 1) * TOTAL_CELLS / THREADS;
        syncThreads[i] = thread(syncCells, std::ref(cells), start, end);
    }
    for (size_t i = 0; i < THREADS; i++) {
        syncThreads[i].join();
    }
}


void drawCells(const vector<Cell> &cells, int divisor, DrawMode drawMode) {
    // A bit exessive to put this outside, but is saves doing CELL_BOUNDS^3 extra checks
    // at the cost of extra code
    switch (drawMode) {
        case DUAL_COLOR:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[threeToOne(x, y, z)].drawDualColor();
                    }
                }
            }
            break;
        case DUAL_COLOR_DYING:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[threeToOne(x, y, z)].drawDualColorDying();
                    }
                }
            }
            break;
        case SINGLE_COLOR:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[threeToOne(x, y, z)].drawSingleColor();
                    }
                }
            }
            break;
        case RGB_CUBE:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[threeToOne(x, y, z)].drawRGBCube();
                    }
                }
            }
            break;
        case CENTER_DIST:
            for (int x = 0; x < CELL_BOUNDS/divisor; x++) {
                for (int y = 0; y < CELL_BOUNDS; y++) {
                    for (int z = 0; z < CELL_BOUNDS; z++) {
                        cells[threeToOne(x, y, z)].drawDist();
                    }
                }
            }
            break;
    }
}


void randomizeCells(vector<Cell> &cells) {
    for (size_t i = 0; i < TOTAL_CELLS; i++) {
        cells[i].randomizeState();
    }
}


void drawLeftBar(bool drawBounds, bool showHalf, bool paused, DrawMode drawMode, TickMode tickMode, float cameraLat, float cameraLon, int updateSpeed) {
    char dirs[2] = {
        (cameraLat > 0 ? 'N' : 'S'),
        (cameraLon > 0 ? 'W' : 'E')
    };

    string survivalText = "- Survive:";
    for (size_t i = 0; i < 27; i++) {
        if (SURVIVAL[i]) survivalText += " " + std::to_string(i);
    }

    string spawnText = "- Spawn:";
    for (size_t i = 0; i < 27; i++) {
        if (SPAWN[i]) spawnText += " " + std::to_string(i);
    }

    const DrawableText dts[] = {
        DrawableText("Controls:"),
        DrawableText("- Q/E : zoom in/out"),
        DrawableText("- W/S : rotate camera up/down"),
        DrawableText("- A/D : rotate camera left/right"),
        DrawableText("- R : re-randomize cells"),
        DrawableText("- B : show/hide bounds " + (string)(drawBounds ? "(on)" : "(off)")),
        DrawableText("- P : show/hide this bar (on)"),
        DrawableText("- C : toggle cross section view " + (string)(showHalf ? "(on)" : "(off)")),
        DrawableText("- Mouse click : pause/unpause " + (string)(paused ? "(paused)" : "(running)")),
        DrawableText("- Space : reset camera"),
        DrawableText("- Enter : toggle fullscreen"),
        DrawableText("- O : toggle true fullscreen (not reccomended)"),
        DrawableText("- M : change between draw modes [" + textFromEnum(drawMode) + "]"),
        DrawableText("- U : change between tick modes [" + textFromEnum(tickMode) + "]"),
        (tickMode == MANUAL ? DrawableText("- X/Z : increase/decrease tick speed") : DrawableText("")),

        DrawableText("Simulation Info:"),
        DrawableText("- FPS: " + std::to_string(GetFPS())),
        DrawableText("- Ticks per sec: " + std::to_string(tickMode == FAST ? GetFPS() : updateSpeed)),
        DrawableText("- Bound size: " + std::to_string(CELL_BOUNDS)),
        DrawableText("- Threads: " + std::to_string(THREADS) + " (+ 2)"),
        DrawableText("- Camera pos: " + std::to_string((int)abs(cameraLat)) + dirs[0] + ", " + std::to_string(abs((int)cameraLon)) + dirs[1]),

        DrawableText("Rules:"),
        DrawableText(survivalText),
        DrawableText(spawnText),
        DrawableText("- State: " + std::to_string(STATE)),
        DrawableText("- Neighborhood: " + textFromEnum(NEIGHBORHOODS)),
    };

    const size_t lenTexts = sizeof(dts) / sizeof(dts[0]);

    DrawRectangle(10, 10, 290, lenTexts * 14 + 7, Fade(SKYBLUE, 0.5f));
    DrawRectangleLines(10, 10, 290, lenTexts * 14 + 7, BLUE);

    for (size_t i = 0; i < lenTexts; i++) {
        dts[i].draw(i);
    }
}


void draw(Camera3D camera, const vector<Cell> &cells, bool drawBounds, bool drawBar, bool showHalf, bool paused, DrawMode drawMode, TickMode tickMode, float cameraLat, float cameraLon, int updateSpeed) {
    BeginDrawing();
        ClearBackground(RAYWHITE);
        BeginMode3D(camera);
            drawCells(cells, (int)showHalf + 1, drawMode);

            if (drawBounds) {
                int outlineSize = CELL_SIZE * CELL_BOUNDS;
                if (showHalf) DrawCubeWires((Vector3){ -outlineSize/4.0f, 0, 0 }, outlineSize/2, outlineSize, outlineSize, BLUE);
                else DrawCubeWires((Vector3){ 0, 0, 0 }, outlineSize, outlineSize, outlineSize, BLUE);
            }
        EndMode3D();
        if (drawBar) {
            drawLeftBar(drawBounds, showHalf, paused, drawMode, tickMode, cameraLat, cameraLon, updateSpeed);
        }
    EndDrawing();
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
    bool drawBounds = false;
    bool showHalf = false;
    bool drawBar = true;
    DrawMode drawMode = DUAL_COLOR;
    TickMode tickMode = FAST;

    ToggleKey mouseTK;
    ToggleKey enterTK;
    ToggleKey xTK;
    ToggleKey zTK;
    ToggleKey bTK;
    ToggleKey cTK;
    ToggleKey mTK;
    ToggleKey uTK;
    ToggleKey pTK;
    ToggleKey oTK;

    int updateSpeed = 8;
    float frame = 0;

    vector<Cell> cells;
    for (int x = 0; x < CELL_BOUNDS; x++) {
        for (int y = 0; y < CELL_BOUNDS; y++) {
            for (int z = 0; z < CELL_BOUNDS; z++) {
                cells.push_back(Cell({ x, y, z }));
            }
        }
    }
    vector<Cell> cells2 = cells;
    cells2.reserve(TOTAL_CELLS);

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
        if (uTK.down(IsKeyDown('U'))) tickMode = (TickMode)((tickMode + 1) % 3);
        if (pTK.down(IsKeyDown('P'))) drawBar = !drawBar;
        if (IsKeyDown(KEY_SPACE)) {
            cameraLat = 20.0f;
            cameraLon = 20.0f;
            cameraRadius = 1.75f * CELL_SIZE * CELL_BOUNDS;
        }
        if (enterTK.down(IsKeyPressed(KEY_ENTER))) {
            if (GetScreenWidth() == screenWidth) MaximizeWindow();
            else RestoreWindow();
        }
        if (oTK.down(IsKeyPressed('O'))) {
 			ToggleFullscreen();
            if (IsWindowFullscreen()) {
                int display = GetCurrentMonitor();
                SetWindowSize(GetMonitorWidth(display), GetMonitorHeight(display));
            }
            else  {
                SetWindowSize(screenWidth, screenHeight);
            }
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

        if (!paused && (tickMode == FAST || frame >= 1.0f/updateSpeed)) {
            if (tickMode == DYNAMIC) {
                if (GetFPS() > TargetFPS && updateSpeed < GetFPS()) updateSpeed++;
                else if (GetFPS() < TargetFPS && updateSpeed > 1) updateSpeed--;
            }
            while (tickMode != FAST && frame >= 1.0/updateSpeed) frame -= 1.0/updateSpeed;

            cells2 = vector<Cell>(cells);
            thread updateThread(updateCells, std::ref(cells2));

            draw(camera, cells, drawBounds, drawBar, showHalf, paused, drawMode, tickMode, cameraLat, cameraLon, updateSpeed);

            updateThread.join();
            cells = vector<Cell>(cells2);
        }
        else {
            draw(camera, cells, drawBounds, drawBar, showHalf, paused, drawMode, tickMode, cameraLat, cameraLon, updateSpeed);
        }

    }

    CloseWindow();        // Close window and OpenGL context
    return 0;
}